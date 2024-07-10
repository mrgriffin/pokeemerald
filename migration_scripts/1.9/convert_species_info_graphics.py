import glob
import re

class Species:
    def __init__(self, line):
        self.graphics = None
        self.femaleGraphics = None
        self.initializers = [line]

    def dump(self):
        dump = ''
        for group in self.initializers:
            if isinstance(group, list):
                dump += ''.join(group)
                dump += '        },\n'
            else:
                dump += group
        return dump

members = ['frontPicSize', 'backPicSize', 'iconPalIndex', 'frontPic', 'backPic', 'palette', 'shinyPalette', 'iconSprite']
graphics_member = re.compile(f' *\.({"|".join(members)})(Female)? *=(.*)')

for path in glob.glob("src/data/pokemon/species_info/*.h"):
    buffer = ''
    species = None
    with open(path) as file:
        for line in file:
            if re.search(r'\[SPECIES_\w+\]', line):
                if species is not None:
                    buffer += species.dump()
                species = Species(line)
            elif species is None:
                buffer += line
            elif m := graphics_member.match(line):
                if m.group(2) == 'Female':
                    attr = 'femaleGraphics'
                    init = '        .femaleGraphics = &(const struct SpeciesGraphics)\n'
                else:
                    attr = 'graphics'
                    init = '        .graphics =\n'
                value = getattr(species, attr)
                if value is None:
                    value = [init, '        {\n']
                    setattr(species, attr, value)
                    species.initializers.append(value)
                value.append(f'            .{m.group(1)} ={m.group(3)}\n')
            else:
                species.initializers.append(line)
                if re.match(r'    }(,)?', line):
                    buffer += species.dump()
                    species = None
        if species is not None:
            buffer += species.dump()
    with open(path, 'w') as file:
        file.write(buffer)
