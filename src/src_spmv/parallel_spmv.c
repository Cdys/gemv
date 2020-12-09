
#include <gemv.h>
void spmv_parallel_Selected(BASIC_INT_TYPE m,
                            const BASIC_INT_TYPE*RowPtr,
                            const BASIC_INT_TYPE *ColIdx,
                            const void *Matrix_Val,
                            const void *Vector_Val_X,
                            void *Vector_Val_Y,
                            BASIC_SIZE_TYPE size,
                            VECTORIZED_WAY vectorizedWay
                   ){
    dot_product_function dotProductFunction = inner_basic_GetDotProduct(size);

#pragma omp parallel default(shared)
    for (int i = 0; i < m; i++) {
        dotProductFunction(RowPtr[i+1]-RowPtr[i],
                           ColIdx+RowPtr[i],Matrix_Val+RowPtr[i]*size,Vector_Val_X,Vector_Val_Y+i*size,
                           vectorizedWay);
    }
}

FUNC_DECLARES(FUNC_NO_HANDLE_DEFINES,parallel);