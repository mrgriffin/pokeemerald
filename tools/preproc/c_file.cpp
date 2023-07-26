// Copyright(c) 2016 YamaArashi
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <cstdio>
#include <cstdarg>
#include <stdexcept>
#include <string>
#include <memory>
#include <cstring>
#include <cerrno>
#include "preproc.h"
#include "c_file.h"
#include "char_util.h"
#include "utf8.h"
#include "string_parser.h"

CFile::CFile(const char * filenameCStr, bool isStdin)
{
    FILE *fp;

    if (isStdin) {
        fp = stdin;
        m_filename = std::string{"<stdin>/"}.append(filenameCStr);
    } else {
        fp = std::fopen(filenameCStr, "rb");
        m_filename = std::string(filenameCStr);
    }

    std::string& filename = m_filename;

    if (fp == NULL)
        FATAL_ERROR("Failed to open \"%s\" for reading.\n", filename.c_str());

    m_size = 0;
    m_buffer = (char *)malloc(CHUNK_SIZE + 1);
    if (m_buffer == NULL) {
        FATAL_ERROR("Failed to allocate memory to process file \"%s\"!", filename.c_str());
    }

    std::size_t numAllocatedBytes = CHUNK_SIZE + 1;
    std::size_t bufferOffset = 0;
    std::size_t count;

    while ((count = std::fread(m_buffer + bufferOffset, 1, CHUNK_SIZE, fp)) != 0) {
        if (!std::ferror(fp)) {
            m_size += count;

            if (std::feof(fp)) {
                break;
            }

            numAllocatedBytes += CHUNK_SIZE;
            bufferOffset += CHUNK_SIZE;
            m_buffer = (char *)realloc(m_buffer, numAllocatedBytes);
            if (m_buffer == NULL) {
                FATAL_ERROR("Failed to allocate memory to process file \"%s\"!", filename.c_str());
            }
        } else {
            FATAL_ERROR("Failed to read \"%s\". (error: %s)", filename.c_str(), std::strerror(errno));
        }
    }

    m_buffer[m_size] = 0;

    std::fclose(fp);

    m_pos = 0;
    m_lineNum = 1;
    m_isStdin = isStdin;
}

CFile::CFile(CFile&& other) : m_filename(std::move(other.m_filename))
{
    m_buffer = other.m_buffer;
    m_pos = other.m_pos;
    m_size = other.m_size;
    m_lineNum = other.m_lineNum;
    m_isStdin = other.m_isStdin;

    other.m_buffer = NULL;
}

CFile::~CFile()
{
    free(m_buffer);
}

void CFile::Preproc()
{
    char stringChar = 0;

    while (m_pos < m_size)
    {
        if (stringChar)
        {
            if (m_buffer[m_pos] == stringChar)
            {
                std::putchar(stringChar);
                m_pos++;
                stringChar = 0;
            }
            else if (m_buffer[m_pos] == '\\' && m_buffer[m_pos + 1] == stringChar)
            {
                std::putchar('\\');
                std::putchar(stringChar);
                m_pos += 2;
            }
            else
            {
                if (m_buffer[m_pos] == '\n')
                    m_lineNum++;
                std::putchar(m_buffer[m_pos]);
                m_pos++;
            }
        }
        else
        {
            TryConvertString();
            TryConvertIncbin();
            TryConvertDSL();

            if (m_pos >= m_size)
                break;

            char c = m_buffer[m_pos++];

            std::putchar(c);

            if (c == '\n')
                m_lineNum++;
            else if (c == '"')
                stringChar = '"';
            else if (c == '\'')
                stringChar = '\'';
        }
    }
}

bool CFile::ConsumeHorizontalWhitespace()
{
    if (m_buffer[m_pos] == '\t' || m_buffer[m_pos] == ' ')
    {
        m_pos++;
        return true;
    }

    return false;
}

bool CFile::ConsumeNewline()
{
    if (m_buffer[m_pos] == '\r' && m_buffer[m_pos + 1] == '\n')
    {
        m_pos += 2;
        m_lineNum++;
        std::putchar('\n');
        return true;
    }

    if (m_buffer[m_pos] == '\n')
    {
        m_pos++;
        m_lineNum++;
        std::putchar('\n');
        return true;
    }

    return false;
}

void CFile::SkipWhitespace()
{
    while (ConsumeHorizontalWhitespace() || ConsumeNewline())
        ;
}

void CFile::TryConvertString()
{
    long oldPos = m_pos;
    long oldLineNum = m_lineNum;
    bool noTerminator = false;

    if (m_buffer[m_pos] != '_' || (m_pos > 0 && IsIdentifierChar(m_buffer[m_pos - 1])))
        return;

    m_pos++;

    if (m_buffer[m_pos] == '_')
    {
        noTerminator = true;
        m_pos++;
    }

    SkipWhitespace();

    if (m_buffer[m_pos] != '(')
    {
        m_pos = oldPos;
        m_lineNum = oldLineNum;
        return;
    }

    m_pos++;

    SkipWhitespace();

    std::printf("{ ");

    while (1)
    {
        SkipWhitespace();

        if (m_buffer[m_pos] == '"')
        {
            unsigned char s[kMaxStringLength];
            int length;
            StringParser stringParser(m_buffer, m_size);

            try
            {
                m_pos += stringParser.ParseString(m_pos, s, length);
            }
            catch (std::runtime_error& e)
            {
                RaiseError(e.what());
            }

            for (int i = 0; i < length; i++)
                printf("0x%02X, ", s[i]);
        }
        else if (m_buffer[m_pos] == ')')
        {
            m_pos++;
            break;
        }
        else
        {
            if (m_pos >= m_size)
                RaiseError("unexpected EOF");
            if (IsAsciiPrintable(m_buffer[m_pos]))
                RaiseError("unexpected character '%c'", m_buffer[m_pos]);
            else
                RaiseError("unexpected character '\\x%02X'", m_buffer[m_pos]);
        }
    }

    if (noTerminator)
        std::printf(" }");
    else
        std::printf("0xFF }");
}

bool CFile::CheckIdentifier(const std::string& ident)
{
    unsigned int i;

    for (i = 0; i < ident.length() && m_pos + i < (unsigned)m_size; i++)
        if (ident[i] != m_buffer[m_pos + i])
            return false;

    return (i == ident.length());
}

std::unique_ptr<unsigned char[]> CFile::ReadWholeFile(const std::string& path, int& size)
{
    FILE* fp = std::fopen(path.c_str(), "rb");

    if (fp == nullptr)
        RaiseError("Failed to open \"%s\" for reading.\n", path.c_str());

    std::fseek(fp, 0, SEEK_END);

    size = std::ftell(fp);

    std::unique_ptr<unsigned char[]> buffer = std::unique_ptr<unsigned char[]>(new unsigned char[size]);

    std::rewind(fp);

    if (std::fread(buffer.get(), size, 1, fp) != 1)
        RaiseError("Failed to read \"%s\".\n", path.c_str());

    std::fclose(fp);

    return buffer;
}

int ExtractData(const std::unique_ptr<unsigned char[]>& buffer, int offset, int size)
{
    switch (size)
    {
    case 1:
        return buffer[offset];
    case 2:
        return (buffer[offset + 1] << 8)
            | buffer[offset];
    case 4:
        return (buffer[offset + 3] << 24)
            | (buffer[offset + 2] << 16)
            | (buffer[offset + 1] << 8)
            | buffer[offset];
    default:
        FATAL_ERROR("Invalid size passed to ExtractData.\n");
    }
}

void CFile::TryConvertIncbin()
{
    std::string idents[6] = { "INCBIN_S8", "INCBIN_U8", "INCBIN_S16", "INCBIN_U16", "INCBIN_S32", "INCBIN_U32" };
    int incbinType = -1;

    for (int i = 0; i < 6; i++)
    {
        if (CheckIdentifier(idents[i]))
        {
            incbinType = i;
            break;
        }
    }

    if (incbinType == -1)
        return;

    int size = 1 << (incbinType / 2);
    bool isSigned = ((incbinType % 2) == 0);

    long oldPos = m_pos;
    long oldLineNum = m_lineNum;

    m_pos += idents[incbinType].length();

    SkipWhitespace();

    if (m_buffer[m_pos] != '(')
    {
        m_pos = oldPos;
        m_lineNum = oldLineNum;
        return;
    }

    m_pos++;

    std::printf("{");

    while (true)
    {
        SkipWhitespace();

        if (m_buffer[m_pos] != '"')
            RaiseError("expected double quote");

        m_pos++;

        int startPos = m_pos;

        while (m_buffer[m_pos] != '"')
        {
            if (m_buffer[m_pos] == 0)
            {
                if (m_pos >= m_size)
                    RaiseError("unexpected EOF in path string");
                else
                    RaiseError("unexpected null character in path string");
            }

            if (m_buffer[m_pos] == '\r' || m_buffer[m_pos] == '\n')
                RaiseError("unexpected end of line character in path string");

            if (m_buffer[m_pos] == '\\')
                RaiseError("unexpected escape in path string");

            m_pos++;
        }

        std::string path(&m_buffer[startPos], m_pos - startPos);

        m_pos++;

        int fileSize;
        std::unique_ptr<unsigned char[]> buffer = ReadWholeFile(path, fileSize);

        if ((fileSize % size) != 0)
            RaiseError("Size %d doesn't evenly divide file size %d.\n", size, fileSize);

        int count = fileSize / size;
        int offset = 0;

        for (int i = 0; i < count; i++)
        {
            int data = ExtractData(buffer, offset, size);
            offset += size;

            if (isSigned)
                std::printf("%d,", data);
            else
                std::printf("%uu,", data);
        }

        SkipWhitespace();

        if (m_buffer[m_pos] != ',')
            break;

        m_pos++;
    }

    if (m_buffer[m_pos] != ')')
        RaiseError("expected ')'");

    m_pos++;

    std::printf("}");
}

void CFile::TryConvertDSL()
{
    long oldPos = m_pos;
    long oldLineNum = m_lineNum;
    long identifierStart, blockStart, blockEnd;
    std::size_t identifierLength;

    if (m_pos > 0 && IsIdentifierChar(m_buffer[m_pos - 1]))
        goto noMatch;

    if (!IsIdentifierStartingChar(m_buffer[m_pos]))
        goto noMatch;

    identifierStart = m_pos;

    do
        m_pos++;
    while (m_pos < m_size && IsIdentifierChar(m_buffer[m_pos]));

    identifierLength = m_pos - identifierStart;

    while (m_pos < m_size)
    {
        if (m_buffer[m_pos] == '\n')
            m_lineNum++;
        if (!IsWhitespace(m_buffer[m_pos]))
            break;
        m_pos++;
    }

    if (!CheckIdentifier("(|"))
        goto noMatch;
    m_pos += 2; // strlen("(|")

    blockStart = m_pos;

    while (true)
    {
        while (true)
        {
            if (m_pos >= m_size)
                RaiseError("expected '|)'");
            if (m_buffer[m_pos] == '|')
                break;
            if (m_buffer[m_pos] == '\n')
                m_lineNum++;
            m_pos++;
        }
        m_pos++;
        if (m_pos >= m_size)
            RaiseError("expected '|)'");
        if (m_buffer[m_pos] == ')')
            break;
    }

    blockEnd = m_pos - 2;
    m_pos++;

    for (auto& dsl : dsls)
    {
        if (std::strncmp(dsl.identifier, &m_buffer[identifierStart], identifierLength) == 0
         && std::strlen(dsl.identifier) == identifierLength)
        {
            (this->*dsl.convert)(blockStart, blockEnd);
            return;
        }
    }

    RaiseError("unknown DSL '%.*s'", identifierLength, &m_buffer[identifierStart]);

noMatch:
    m_pos = oldPos;
    m_lineNum = oldLineNum;
}

struct Match
{
    Match() : m_start(0), m_end(0) {}
    Match(long start, long end) : m_start(start), m_end(end) {}

    long m_start;
    long m_end;

    int Size() const
    {
        return m_end - m_start;
    }

    operator bool() const
    {
        return m_start < m_end;
    }
};

// Whitespace-insensitive parser.
class TrimParser
{
public:
    TrimParser(const char *buffer, long start, long end) : m_buffer(buffer), m_pos(start), m_end(end) { SkipWhitespace(); }

    TrimParser ParseLine()
    {
        long start = m_pos;
        while (m_pos < m_end && m_buffer[m_pos] != '\n')
            m_pos++;
        long end = m_pos++;
        return TrimParser(m_buffer, start, end);
    }

    TrimParser ParseBlock()
    {
        long start = m_pos;
        while (m_pos < m_end)
        {
            if (m_buffer[m_pos] == '\n')
            {
                if (m_pos + 1 < m_end && m_buffer[m_pos + 1] == '\n')
                {
                    m_pos += 1;
                    break;
                }
                if (m_pos + 2 < m_end && m_buffer[m_pos + 1] == '\r' && m_buffer[m_pos + 2] == '\n')
                {
                    m_pos += 2;
                    break;
                }
            }
            m_pos++;
        }
        long end = m_pos++;
        return TrimParser(m_buffer, start, end);
    }

    void SkipWhitespace()
    {
        while (m_pos < m_end && IsWhitespace(m_buffer[m_pos]))
            m_pos++;
    }

    Match MatchLiteral(const char *string)
    {
        SkipWhitespace();
        long start = m_pos, pos = m_pos;
        for (; pos < m_end; pos++, string++)
        {
            if (*string != m_buffer[pos])
                break;
        }

        if (*string == '\0')
        {
            m_pos = pos;
            return Match(start, m_pos);
        }
        else
        {
            return Match();
        }
    }

    Match MatchUntil(const char *terminators)
    {
        long start = m_pos;
        while (m_pos < m_end && !std::strchr(terminators, m_buffer[m_pos]))
            m_pos++;
        long end = m_pos;

        while (start < end && IsWhitespace(m_buffer[start]))
            start++;
        while (end > start && IsWhitespace(m_buffer[end - 1]))
            end--;

        return Match(start, end);
    }

    Match MatchBetween(char first, char last)
    {
        SkipWhitespace();
        long start = m_pos;
        long pos = m_pos;
        if (pos < m_end && m_buffer[pos] == first)
        {
            while (pos < m_end)
            {
                if (m_buffer[pos] == last)
                {
                    m_pos = pos + 1;
                    return Match(start + 1, pos);
                }
                pos++;
            }
        }
        return Match();
    }

    operator bool() const
    {
        return m_pos < m_end;
    }

private:
    const char *m_buffer;
    long m_pos;
    long m_end;
};

static void PrintConstant(const char *prefix, const char *suffix, const char *buffer, Match match)
{
    std::printf("%s", prefix);
    const char *constant = &buffer[match.m_start];
    for (int i = 0; i < match.Size(); i++)
    {
        if (('A' <= constant[i] && constant[i] <= 'Z')
         || ('0' <= constant[i] && constant[i] <= '9'))
        {
            std::putchar(constant[i]);
        }
        else if ('a' <= constant[i] && constant[i] <= 'z')
        {
            std::putchar(constant[i] - 'a' + 'A');
        }
        else
        {
            std::putchar('_');
        }
    }
    std::printf("%s", suffix);
}

static void PrintNumberDefault(const char *buffer, Match match, int default_)
{
    if (match)
        std::printf("%.*s", match.Size(), &buffer[match.m_start]);
    else
        std::printf("%d", default_);
}

void CFile::ConvertCustomizedParty(long start, long end)
{
    std::printf("{");

    auto p = TrimParser{m_buffer, start, end};

    while (true)
    {
        bool firstMove = true;

        auto b = p.ParseBlock();
        if (!b)
            break;

        std::printf("{");

        auto l = b.ParseLine();

        // Species
        // Species @ Item
        // Species (Gender) @ Item
        // Nickname (Species)
        // Nickname (Species) @ Item
        // Nickname (Species) (Gender) @ Item
        Match nickname, species, gender, item;

        auto first = l.MatchUntil("(@");
        auto second = l.MatchBetween('(', ')');
        auto third = l.MatchBetween('(', ')');

        if (first && second && third)
        {
            nickname = first;
            species = second;
            gender = third;
        }
        else if (first && second)
        {
            if (second.Size() == 1)
            {
                species = first;
                gender = second;
            }
            else
            {
                nickname = first;
                species = second;
            }
        }
        else // first.
        {
            species = first;
        }

        if (l.MatchLiteral("@"))
            item = l.MatchUntil("\n");

        if (nickname)
        {
            std::printf(".nickname = (const u8[]) {");
            std::vector<char> buffer;
            buffer.push_back('\"');
            for (int i = 0; i < nickname.Size(); i++)
                buffer.push_back(m_buffer[nickname.m_start + i]);
            buffer.push_back('\"');
            StringParser sp(buffer.data(), buffer.size());
            unsigned char encoded[kMaxStringLength];
            int length = 0, encodedLength;
            try
            {
                length = sp.ParseString(0, encoded, encodedLength);
            }
            catch (std::runtime_error& e)
            {
                RaiseError(e.what());
            }
            if (length != (int)buffer.size())
                RaiseError("nickname: unconsumed characters");
            for (int i = 0; i < encodedLength; i++)
                std::printf("0x%02X,", encoded[i]);
            std::printf("0xFF},");
        }

        // TODO: Smogon format uses, e.g. Meowth-Alola, but our
        // constants are SPECIES_MEOWTH_ALOLAN.
        PrintConstant(".species = SPECIES_", ",", m_buffer, species);

        if (gender)
        {
            switch (m_buffer[gender.m_start])
            {
            case 'M':
                std::printf(".gender = TRAINER_MON_MALE,");
                break;

            case 'F':
                std::printf(".gender = TRAINER_MON_FEMALE,");
                break;

            default:
                RaiseError("unknown gender '%.*s'", gender.Size(), &m_buffer[gender.m_start]);
            }
        }

        if (item)
            PrintConstant(".heldItem = ITEM_", ",", m_buffer, item);

        // Attribute: Value
        // Value Nature
        while ((l = b.ParseLine()))
        {
            Match isEVs, isIVs;

            if (l.MatchLiteral("-"))
                goto parseMove;

            if (l.MatchLiteral("Ability:"))
            {
                auto ability = l.MatchUntil("\n");
                PrintConstant(".ability = ABILITY_", ",", m_buffer, ability);
            }
            else if (l.MatchLiteral("Ball:"))
            {
                auto ball = l.MatchUntil("\n");
                PrintConstant(".ball = ITEM_", ",", m_buffer, ball);
            }
            else if ((isEVs = l.MatchLiteral("EVs:"))
                  || (isIVs = l.MatchLiteral("IVs:")))
            {
                Match hp, atk, def, spa, spd, spe;
                while (true)
                {
                    l.SkipWhitespace();
                    if (!l)
                        RaiseError("missing stats");
                    auto number = l.MatchUntil(" ");
                    if (l.MatchLiteral("HP"))
                    {
                        hp = number;
                    }
                    else if (l.MatchLiteral("Atk"))
                    {
                        atk = number;
                    }
                    else if (l.MatchLiteral("Def"))
                    {
                        def = number;
                    }
                    else if (l.MatchLiteral("SpA"))
                    {
                        spa = number;
                    }
                    else if (l.MatchLiteral("SpD"))
                    {
                        spd = number;
                    }
                    else if (l.MatchLiteral("Spe"))
                    {
                        spe = number;
                    }
                    else
                    {
                        auto name = l.MatchUntil("/\n");
                        RaiseError("unknown stat '%.*s'", name.Size(), &m_buffer[name.m_start]);
                    }
                    if (!l.MatchLiteral("/"))
                        break;
                }
                if (isEVs)
                    std::printf(".ev = TRAINER_PARTY_EVS(");
                else
                    std::printf(".iv = TRAINER_PARTY_IVS(");
                PrintNumberDefault(m_buffer, hp, 0); std::putchar(',');
                PrintNumberDefault(m_buffer, atk, 0); std::putchar(',');
                PrintNumberDefault(m_buffer, def, 0); std::putchar(',');
                PrintNumberDefault(m_buffer, spe, 0); std::putchar(',');
                PrintNumberDefault(m_buffer, spa, 0); std::putchar(',');
                PrintNumberDefault(m_buffer, spd, 0);
                std::printf("),");
            }
            else if (l.MatchLiteral("Happiness:"))
            {
                auto happiness = l.MatchUntil("\n");
                PrintConstant(".friendship = ", ",", m_buffer, happiness);
            }
            else if (l.MatchLiteral("Level:"))
            {
                auto level = l.MatchUntil("\n");
                PrintConstant(".lvl = ", ",", m_buffer, level);
            }
            else if (l.MatchLiteral("Shiny:"))
            {
                if (l.MatchLiteral("Yes"))
                {
                    std::printf(".isShiny = TRUE,");
                }
                else if (l.MatchLiteral("No"))
                {
                    std::printf(".isShiny = FALSE,");
                }
                else
                {
                    auto shininess = l.MatchUntil("\n");
                    RaiseError("unexpected shininess: '%.*s'", shininess.Size(), &m_buffer[shininess.m_start]);
                }
            }
            else
            {
                auto nature = l.MatchUntil(" ");
                if (!l.MatchLiteral("Nature"))
                    RaiseError("expected Nature");
                PrintConstant(".nature = TRAINER_PARTY_NATURE(NATURE_", "),", m_buffer, nature);
            }
        }

        // - Move
        while ((l = b.ParseLine()))
        {
            if (!l.MatchLiteral("-"))
                break;
parseMove:
            if (firstMove)
            {
                std::printf(".moves = {");
                firstMove = false;
            }
            auto move = l.MatchUntil("\n");
            PrintConstant("MOVE_", ",", m_buffer, move);
        }
        if (!firstMove)
            std::printf("}");

        std::printf("},");
    }

    std::printf("}");
}

// Reports a diagnostic message.
void CFile::ReportDiagnostic(const char* type, const char* format, std::va_list args)
{
    const int bufferSize = 1024;
    char buffer[bufferSize];
    std::vsnprintf(buffer, bufferSize, format, args);
    std::fprintf(stderr, "%s:%ld: %s: %s\n", m_filename.c_str(), m_lineNum, type, buffer);
}

#define DO_REPORT(type)                   \
do                                        \
{                                         \
    std::va_list args;                    \
    va_start(args, format);               \
    ReportDiagnostic(type, format, args); \
    va_end(args);                         \
} while (0)

// Reports an error diagnostic and terminates the program.
void CFile::RaiseError(const char* format, ...)
{
    DO_REPORT("error");
    std::exit(1);
}

// Reports a warning diagnostic.
void CFile::RaiseWarning(const char* format, ...)
{
    DO_REPORT("warning");
}
