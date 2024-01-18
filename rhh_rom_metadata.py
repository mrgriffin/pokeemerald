import collections
import struct

class Slice:
    def __init__(self, region, begin, end):
        self.region = region
        self.begin = begin
        self.end = end

    def __repr__(self):
        return f"{self.region}[0x{self.begin:07x}:0x{self.end:07x}]"

class Array:
    def __init__(self, values):
        self.values = values

    def __repr__(self):
        return repr(self.values)

class VM:
    def __init__(self, rom):
        self.rom = rom

        header_off = rom.index(b'RHHEXP')
        if header_off is None:
            raise Exception("Missing RHH ROM header")
        (program_addr, program_size) = struct.unpack_from('<II', buffer=self.rom, offset=header_off + 0x14)
        program_off = program_addr - 0x8000000
        self.program = self.rom[program_off:program_off + program_size]

        self.ip_stack = [0]
        self.data_stack = []

    # TODO: Human-usable API:
    # Take operation and validate inputs/outputs with it.
    # Return (expected) outputs.
    def execute(self, *args):
        self.data_stack = list(reversed(args))
        while self.ip_stack:
            op = self.read_ip('B')
            getattr(self, f"execute_{op:02x}")()

    def read_ip(self, fmt):
        fmt = '<' + fmt
        (read,) = struct.unpack_from(fmt, buffer=self.program, offset=self.ip_stack[-1])
        self.ip_stack[-1] += struct.calcsize(fmt)
        return read

    def execute_00(self):
        """ret ( -- )"""
        self.ip_stack.pop()

    def execute_01(self):
        """u8 ( -- n)"""
        self.data_stack.append(self.read_ip('B'))

    def execute_02(self):
        """s8 ( -- :i)"""
        self.data_stack.append(self.read_ip('b'))

    def execute_03(self):
        """u16 ( -- :i)"""
        self.data_stack.append(self.read_ip('H'))

    def execute_04(self):
        """s16 ( -- :i)"""
        self.data_stack.append(self.read_ip('h'))

    def execute_05(self):
        """i32 ( -- :i)"""
        self.data_stack.append(self.read_ip('I'))

    def execute_06(self):
        """dup (:a -- :a :a)"""
        self.data_stack.append(self.data_stack[-1])

    def execute_07(self):
        """swap (:a :b -- :b :a)"""
        rhs = self.data_stack.pop()
        lhs = self.data_stack.pop()
        self.data_stack.append(rhs)
        self.data_stack.append(lhs)

    def execute_08(self):
        """from-pointer (address:i -- :s)"""
        offset = self.data_stack.pop() - 0x8000000
        self.data_stack.append(Slice('rom', offset, 0xA000000 - offset))

    def execute_09(self):
        """index (:s index:i size:i -- :s)"""
        size = self.data_stack.pop()
        index = self.data_stack.pop()
        slice_ = self.data_stack.pop() # TODO: pop_slice etc.
        begin = slice_.begin + index * size
        end = begin + size
        assert slice_.begin <= begin <= slice_.end
        assert slice_.begin <= end <= slice_.end
        self.data_stack.append(Slice(slice_.region, begin, end))

    def execute_0a(self):
        """offset (:s offset:i -- :s)"""
        offset = self.data_stack.pop()
        slice_ = self.data_stack.pop()
        begin = slice_.begin + offset
        assert slice_.begin <= begin <= slice_.end
        self.data_stack.append(Slice(slice_.region, begin, slice_.end))

    # TODO: load-u8, load-u16, load-s8, load-s16.
    def execute_0b(self):
        """load-32 (:s -- :i)"""
        slice_ = self.data_stack.pop()
        assert slice_.begin + 4 <= slice_.end
        region = getattr(self, slice_.region)
        (_32,) = struct.unpack_from('<I', buffer=region, offset=slice_.begin)
        self.data_stack.append(_32)

    # TODO: load-terminated of other sizes.
    # TODO: load-count.
    # TODO: load-terminated-count.
    def execute_0c(self):
        """load-terminated-8s (:s terminator:i -- :a)"""
        terminator = self.data_stack.pop() & 0xFF
        slice_ = self.data_stack.pop()
        region = getattr(self, slice_.region)
        values = []
        i = slice_.begin
        while True:
            if i >= slice_.end:
                raise Exception("Unterminated array")
            if region[i] == terminator:
                break
            values.append(region[i])
            i += 1
        self.data_stack.append(Array(values))

if __name__ == '__main__':
    with open("pokeemerald.gba", "rb") as f:
        rom = f.read()
    vm = VM(rom)
    vm.execute(22) # 22 = ABILITY_INTIMIDATE
    print(vm.data_stack)
