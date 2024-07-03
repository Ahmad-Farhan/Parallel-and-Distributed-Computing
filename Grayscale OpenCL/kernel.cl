__kernel void convert(__global const uchar* red,
                      __global const uchar* green,
                      __global const uchar* blue,
				      __global uchar* gray,
                      __global uint *w){

    const int idx = get_global_id(0);
    const int size = (int)(*w);
    const int row = idx * size;

    for(int i = 0; i < size; i++){
        float pixel = 0.299f * (float)(*(red + row + i));
        pixel += 0.587f * (float)(*(green + row + i));
        pixel += 0.114f * (float)(*(blue + row + i));
        *(gray + row + i) = (uchar)pixel;
    }
}
