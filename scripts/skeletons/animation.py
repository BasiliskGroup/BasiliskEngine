import glm

class Animation():
    
    def __init__(self, key_frames:list):
        self.key_frames = key_frames
        self.current_key_frame = 0
        self.time = 0
        self.is_running = False
        
    def update(self, delta_time:float):
        """
        Updates animation key_frame
        """
        if not self.is_running: return 
        
        self.time += delta_time
        if self.time > self.key_frames[self.current_key_frame].time: 
            self.current_key_frame += 1
            self.time               = 0
            
            if self.current_key_frame >= len(self.key_frames): self.reset()
        
    def get_key_frame(self, offset = 0) -> tuple:
        """
        Gets the position, rotation, and remaining time for the current animation key_frame
        """
        if not 0 <= self.current_key_frame + offset < len(self.key_frames): return None, None, None
        return self.key_frames[self.current_key_frame + offset].position, self.key_frames[self.current_key_frame + offset].rotation, self.key_frames[self.current_key_frame + offset].time - self.time
        
    def reset(self):
        """
        Resets animation to original state
        """
        self.current_key_frame = 0
        self.is_running = False
        self.time = 0
        
# struct like
class KeyFrame():
    
    def __init__(self, position:glm.vec3=None, rotation:glm.quat=None, time:float=1):
        self.position = glm.vec3(position) if position else position
        self.rotation = glm.normalize(glm.quat(rotation)) if rotation else rotation
        self.time     = time