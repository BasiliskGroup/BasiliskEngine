import glm

self.overlay_time += self.engine.dt

# Center reticle
self.overlay.draw_circle(0, 0, .0075, color=(225, 225, 225), win_size=self.engine.win_size)
self.overlay.draw_circle(0, 0, .01, color=(15, 15, 15), win_size=self.engine.win_size)

if self.engine.mouse_buttons[0]:
    mouse_position = (self.engine.mouse_position[0] / self.engine.win_size[0] * 2 - 1, self.engine.mouse_position[1] / self.engine.win_size[1] * 2 - 1)
    self.overlay.draw_circle(*mouse_position, .0075, color=(225, 225, 225), win_size=self.engine.win_size)
    self.overlay.draw_circle(*mouse_position, .01, color=(15, 15, 15), win_size=self.engine.win_size)


    n_lines = 20
    speed = 2.0
    gap = 0.5

    delta_position = glm.normalize(glm.vec2(glm.vec2(mouse_position) - glm.vec2(0, 0))) / 25
    line_start = delta_position * ((self.overlay_time * speed) % 1.0) * (1 + gap)

    if glm.length(line_start) > glm.length(delta_position) * gap:
        self.overlay.draw_line((0, 0), line_start - delta_position * gap, thickness=0.002, color=(225, 225, 225))

    while glm.length(line_start + delta_position) < glm.length(glm.vec2(mouse_position)):
        self.overlay.draw_line(line_start, line_start + delta_position, thickness=0.002, color=(225, 225, 225))
        line_start += delta_position * (1 + gap)

    if glm.length(line_start) < glm.length(glm.vec2(mouse_position)):
        self.overlay.draw_line(line_start, mouse_position, thickness=0.002, color=(225, 225, 225))

self.overlay.render()