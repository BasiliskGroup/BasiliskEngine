class Parent:
    def __init__(self) -> None:
        self.child = Child()
        self.x = 0


class Child:
    def __init__(self) -> None:
        self.y = 0


parent = Parent()

setattr(parent.child, "y", 4)
print(parent.child.y)