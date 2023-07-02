import pytest
from miniraytracer import vec3


class Test_vec3:
    
    def test_multiply(self):
        v1 = vec3(1, 2, 3)
        v2 = vec3(0, 0, 1)
        assert v1 * v2 == 3

    