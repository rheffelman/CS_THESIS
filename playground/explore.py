"""explore(obj): prints what kind of object something is, and everything it has.

Usage, anywhere in main.py:
    from explore import explore
    explore(tree)
    explore(tree.statement(0).expr().operand(0))
"""
import inspect


def short(value):
    """A readable one-line description of a value."""
    if isinstance(value, list):
        return "[" + ", ".join(short(v) for v in value) + "]"
    if hasattr(value, "getText") and hasattr(value, "getChildCount"):  # a tree node
        text = value.getText()
        text = text if len(text) <= 30 else text[:27] + "..."
        return f"<{type(value).__qualname__} {text!r}>"
    if type(value).__name__ == "CommonToken":
        return f"<Token {value.text!r} line {value.line} col {value.column}>"
    if type(value).__name__ == "TestParser":
        return "<the TestParser>"
    text = repr(value)
    return text if len(text) <= 70 else text[:67] + "..."


def explore(obj):
    cls = type(obj)
    print(f"\n=== {cls.__qualname__}")
    print("is a:", " -> ".join(c.__qualname__ for c in cls.__mro__ if c is not object))

    # Attributes: data stored on this particular object.
    names = set(getattr(obj, "__dict__", {}))
    for c in cls.__mro__:
        slots = getattr(c, "__slots__", ())
        names.update([slots] if isinstance(slots, str) else slots)
        names.update(n for n, m in vars(c).items() if isinstance(m, property))  # e.g. token.text
    print("\nattributes:")
    for name in sorted(n for n in names if not n.startswith("_")):
        if hasattr(obj, name):
            print(f"    .{name} = {short(getattr(obj, name))}")

    # Methods, grouped by the class that defines them (most specific first).
    for c in cls.__mro__:
        methods = []
        for name, member in vars(c).items():
            if name.startswith("_") or not callable(member):
                continue
            try:
                sig = str(inspect.signature(member)).replace("(self, ", "(").replace("(self)", "()")
            except (TypeError, ValueError):
                sig = "(...)"
            methods.append(name + sig)
        if methods and c is not object:
            where = "generated from your grammar" if c.__module__.startswith("generated") else "ANTLR runtime"
            print(f"\nmethods from {c.__qualname__}  [{where}]:")
            for m in sorted(methods):
                print("    ." + m)
