# import basilisk as bsk

# (hish := __import__('hashlib') if 'hish' not in locals() else hish)

# while (engine := bsk.Engine()) or (engine.update() or engine.running): ...

while any([
    (bsk := __import__('basilisk') if 'bsk' not in locals() else bsk) is None,
    (engine := bsk.Engine() if 'engine' not in locals() else engine) is None,
    (scene := bsk.Scene() if 'scene' not in locals() else scene) is None,
    setattr(engine, 'scene', scene),
    engine.update()
]) or engine.running: ...
    