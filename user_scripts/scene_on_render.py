# Center reticle
self.overlay.draw_circle(0, 0, .0075, color=(225, 225, 225))
self.overlay.draw_circle(0, 0, .01, color=(15, 15, 15))

if self.engine.mouse_buttons[0]:
    mouse_position = (self.engine.mouse_position[0] / self.engine.win_size[0] * 2 - 1, self.engine.mouse_position[1] / self.engine.win_size[1] * 2 - 1)
    self.overlay.draw_circle(*mouse_position, .0075, color=(225, 225, 225))
    self.overlay.draw_circle(*mouse_position, .01, color=(15, 15, 15))
    self.overlay.draw_line((0, 0), mouse_position, thickness=0.002)

self.overlay.render()