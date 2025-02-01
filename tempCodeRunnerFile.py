update()
    
    if engine.keys[bsk.pg.K_1]:
        scene.particle_handler.add()

    if engine.keys[bsk.pg.K_2]:
        scene.particle_handler.add(color=(225, 50, 50))

    if engine.keys[bsk.pg.K_3]:
        scene.particle_handler.add(velocity=(random.randrange(-5, 5) for i in range(3)))
    
    if engine.keys[bsk.pg.K_4]:
        scene.particle_handler.add(velocity=(0, 0, 0), acceleration=(random.randrange(-5, 5) for i in range(3)))