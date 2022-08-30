void listout(int a[], int size, int lb = 0, int ub = -1, int pivot_index = -1) {
    if(ub == -1) ub = size - 1;
    std::cout << "[ ";
    for(int i = lb; i <= ub; i++) if(i == pivot_index) std::cout << "(" << a[i] << ") "; else std::cout << a[i] << " ";
    std::cout << "]\n";
}