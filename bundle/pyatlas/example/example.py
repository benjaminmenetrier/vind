import pyatlas

def add_fields(fs):
    # ndarray_view returns numpy views onto the atlas field memory
    # (no copy), so writing into `c` updates field C seen by C++.
    a = pyatlas.ndarray_view(fs["A"])
    b = pyatlas.ndarray_view(fs["B"])
    c = pyatlas.ndarray_view(fs["C"])
    print(fs)
    print("A:", a)
    print("B:", b)
    c[:] = a + b
    print(f"C (from python): {c}", flush=True)
