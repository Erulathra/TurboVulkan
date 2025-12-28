import gdb

class FNamePrinter:
    def __init__(self, val):
        self.val = val
    def to_string(self):
        expr = f'((Turbo::FName*) {int(self.val.address)})->ToString()'
        return gdb.parse_and_eval(expr)

class FHandlePrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        is_valid = gdb.parse_and_eval(f'((Turbo::FHandle*) {int(self.val.address)})->IsValid()')

        if is_valid:
            index = gdb.parse_and_eval(f'((Turbo::FHandle*) {int(self.val.address)})->GetIndex()')
            generation = gdb.parse_and_eval(f'((Turbo::FHandle*) {int(self.val.address)})->GetGeneration()')

            return f'Index: {index}, Gen: {generation}'

        return 'Invalid'
def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("fname_pp")
    pp.add_printer('Turbo::FName', '^Turbo::FName$', FNamePrinter)

    pp = gdb.printing.RegexpCollectionPrettyPrinter("fhandle_pp")
    pp.add_printer('Turbo::FHandle', r'^Turbo::FHandle$', FHandlePrinter)
    pp.add_printer('Turbo::THandle', r'^Turbo::THandle<', FHandlePrinter)

    return pp

gdb.printing.register_pretty_printer(
    gdb.current_objfile(),
    build_pretty_printer())
