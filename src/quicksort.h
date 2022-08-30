#define DEBUGQ 0

void partition(int a[], int size, int lb, int ub, int & pivot_index) {
    int bound = lb; //lower half's boundary
    if(DEBUGQ) std::cout << "  PA: " << lb << " to " << ub << std::endl;
    for(int i = lb; i <= ub; i++) {
        if(DEBUGQ) std::cout << "  comparing: " << a[i] << " and " << a[pivot_index] << std::endl;
        if(a[i] < a[pivot_index]) {
            if(i == bound) {
                bound++;
                continue; //no need to swap
            }
            //swap
            if(DEBUGQ) std::cout << "  swap!" << std::endl;
            int t = a[i];
            a[i] = a[bound];
            a[bound] = t;
            //did we swap the pivot out of the lower portion?
            //(nb: we cannot be swapping IN the pivot because a[i] == a[pivot_index])
            if(bound == pivot_index) {
                pivot_index = i;
                if(DEBUGQ) std::cout << "  pivot was swapped out! new index: " << pivot_index << std::endl;
            }
            //reflect growth of lower portion
            bound++;
        }
    }
    //put the pivot at the right place
    int t = a[pivot_index];
    a[pivot_index] = a[bound];
    a[bound] = t;
    pivot_index = bound;
    //the array should now be: [lower (0...lb-1)] pivot (lb) [upper (lb+1...n-1)]
    return;
}

void quicksort(int a[], int size, int lb = 0, int ub = -1) {

    if(ub == -1) ub = size - 1; //hehe

    if(DEBUGQ) std::cout << "QS: " << lb << " to " << ub << std::endl;

    //base
    if(ub - lb < 1) {
        if(DEBUGQ) std::cout << "Base case (1)" << std::endl;
        return;
    }
    if(ub - lb == 1) {
        if(DEBUGQ) std::cout << "Base case (2)" << std::endl;
        if(a[lb] < a[ub]) return;
        int t = a[lb];
        a[lb] = a[ub];
        a[ub] = t;
        return;
    }

    //divide
    int pivot_index = lb + (std::rand() % (ub - lb + 1));
    if(DEBUGQ) std::cout << "Pivot is index " << pivot_index << " (" << a[pivot_index] << "):" << std::endl;
    if(DEBUGQ) listout(a, size, lb, ub, pivot_index);
    partition(a, size, lb, ub, pivot_index);

    if(DEBUGQ) std::cout << "Divide (Partition): " << std::endl;
    if(DEBUGQ) listout(a, size, lb, ub, pivot_index);

    //recurse
    if(lb < pivot_index - 1) quicksort(a, size, lb, pivot_index - 1);
    if(ub > pivot_index + 1) quicksort(a, size, pivot_index + 1, ub);

    //conquer (no need)
    if(DEBUGQ) std::cout << "Conquer: " << std::endl;
    if(DEBUGQ) listout(a, size, lb, ub);
    return;
}