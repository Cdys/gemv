//
// Created by kouushou on 2020/11/25.
//

#include <gemv.h>
#include <math.h>

int binary_search_right_boundary_kernel(const int *row_pointer,
                                        const int  key_input,
                                        const int  size)
{
    int start = 0;
    int stop  = size - 1;
    int median;
    int key_median;

    while (stop >= start)
    {
        median = (stop + start) / 2;

        key_median = row_pointer[median];

        if (key_input >= key_median)
            start = median + 1;
        else
            stop = median - 1;
    }

    return start;
}

void parallel_balanced_get_handle(
        gemv_Handle_t* handle,
        BASIC_INT_TYPE m,
        const BASIC_INT_TYPE*RowPtr,
        BASIC_INT_TYPE nnzR,
        BASIC_INT_TYPE nthreads) {
    int *csrSplitter = (int *) malloc((nthreads + 1) * sizeof(int));
    //int *csrSplitter_normal = (int *)malloc((nthreads+1) * sizeof(int));

    int stridennz = ceil((double) nnzR / (double) nthreads);

#pragma omp parallel default(none) shared(nthreads, stridennz, nnzR, RowPtr, csrSplitter, m)
    for (int tid = 0; tid <= nthreads; tid++) {
        // compute partition boundaries by partition of size stride
        int boundary = tid * stridennz;
        // clamp partition boundaries to [0, nnzR]
        boundary = boundary > nnzR ? nnzR : boundary;
        // binary search
        csrSplitter[tid] = binary_search_right_boundary_kernel(RowPtr, boundary, m + 1) - 1;
    }
    *handle = gemv_create_handle();
    (*handle)->nthreads = nthreads;
    (*handle)->status = STATUS_BALANCED;
    (*handle)->csrSplitter = csrSplitter;

}


void spmv_parallel_balanced_Selected(
        const gemv_Handle_t handle,
        BASIC_INT_TYPE m,
        const BASIC_INT_TYPE* RowPtr,
        const BASIC_INT_TYPE* ColIdx,
        const void* Matrix_Val,
        const void* Vector_Val_X,
        void*       Vector_Val_Y,
        BASIC_SIZE_TYPE size,
        VECTORIZED_WAY way
) {
    if(handle->status != STATUS_BALANCED) {
        return;
    }

    dot_product_function dotProductFunction = inner_basic_GetDotProduct(size);

    const int *csrSplitter = handle->csrSplitter;
    const int nthreads = handle->nthreads;
    {
#pragma omp parallel for
        for (int tid = 0; tid < nthreads; tid++) {
            for (int u = csrSplitter[tid]; u < csrSplitter[tid + 1]; u++) {
                dotProductFunction(RowPtr[u + 1] - RowPtr[u], ColIdx + RowPtr[u],
                                     Matrix_Val + RowPtr[u]*size, Vector_Val_X,Vector_Val_Y+u*size,way);
            }
        }
    }
}




FUNC_DECLARES(FUNC_HANDLE_DEFINES,parallel_balanced);