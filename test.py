def test(*args: int):
    for thing in args:
        print(thing)

test(1, 2, 3)