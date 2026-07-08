import sys
import os

def test_api():
    try:
        import modalith
    except ImportError as e:
        print("modalith module not found. Make sure it is in PYTHONPATH.")
        print(e)
        return

    print("Modalith module imported successfully")
    
    # Test Vec3
    v = modalith.Vec3(1.0, 2.0, 3.0)
    assert v.x == 1.0 and v.y == 2.0 and v.z == 3.0
    
    # Test Ray
    ray = modalith.Ray()
    ray.origin = modalith.Vec3(0, 0, 0)
    ray.direction = modalith.Vec3(0, 0, 1)
    
    # Test System
    sys_opt = modalith.OpticalSystem()
    sys_opt.title = "Test System"
    assert sys_opt.title == "Test System"
    
    # Test SurfaceType enum
    assert modalith.SurfaceType.Sphere != modalith.SurfaceType.Plane
    
    # Test non-sequential
    scene = modalith.ns.Scene()
    entity = scene.create_entity("LightSource")
    assert isinstance(entity, int)
    
    print("All Python API basic tests passed!")

if __name__ == "__main__":
    test_api()
