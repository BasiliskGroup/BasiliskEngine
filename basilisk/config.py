import glm
from .generic.input_validation import validate_glm_float, validate_int
from typing import Any


class Config():
    engine: Any
    """Reference to the parent engine"""
    chunk_size: int
    """The size of the chunks in the chunk handler"""
    render_distance: int
    """The number of chunks that are rendered around the camera"""
    bloom_enabled: bool
    """Flag for bloom to be enabled or disabled"""
    bloom_quality: int
    """Quality of the bloom. Suggested values from 2-8"""
    gamma: glm.float32
    """The global gamma level"""
    exposure: glm.float32
    """The global HDR exposure level"""

    def __init__(self, engine) -> None:
        # Reference to the engine
        self.engine = engine
        
        # Chunk configs
        self.chunk_size = 40
        self.render_distance = 5
        
        # Render configs
        self.bloom_enabled  = True
        self._bloom_quality = 6
        self.bloom_quality  = 6
        self.gamma          = 2.2
        self.exposure       = 1.0

    @property
    def bloom_quality(self): return self._bloom_quality
    @property
    def gamma(self): return self._gamma
    @property
    def exposure(self): return self._exposure

    @bloom_quality.setter
    def bloom_quality(self, value):
        if self._bloom_quality == value: return
        self._bloom_quality = validate_int('engine.config', 'bloom quality', value)
        self.engine.frame.bloom.generate_bloom_buffers()
    @gamma.setter
    def gamma(self, value):
        self._gamma = validate_glm_float('engine.config', 'gamma', value)
    @exposure.setter
    def exposure(self, value):
        self._exposure = validate_glm_float('engine.config', 'exposure', value)