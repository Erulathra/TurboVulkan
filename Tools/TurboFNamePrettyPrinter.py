import gdb

class FNamePrinter:
    def __init__(self, val):
        self.val = val
    def to_string(self):
        expr = f'((Turbo::FName*) {int(self.val.address)})->ToString()'
        return gdb.parse_and_eval(expr)

def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("fname_pp")
    pp.add_printer('Turbo::FName', '^Turbo::FName$', FNamePrinter)
    return pp

gdb.printing.register_pretty_printer(
    gdb.current_objfile(),
    build_pretty_printer())
