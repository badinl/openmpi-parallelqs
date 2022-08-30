#include<cstdlib>
#define DEBUGM 0

/// standard O(n+m) algorithm to merge two sorted sequences
int * binary_merge(int* a, int size_a, int* b, int size_b) {
    if(DEBUGM) std::cerr << "Binary merge - a ptr: " << a << " - a size: " << size_a << " - b ptr: " << b << " - b size: " << size_b << std::endl;
    int * out = (int*)malloc(sizeof(int) * (size_a + size_b));
    if(DEBUGM) std::cerr << "Allocated " << sizeof(int) * (size_a + size_b) << " for output at " << out << std::endl;

    int i = 0, j = 0;
    while (i < size_a && j < size_b) {
        if(a[i] < b[j]) {
            out[i+j] = a[i];
            i++;
        } else {
            out[i+j] = b[j];
            j++;
        }
    }
    if(DEBUGM) std::cerr << "Exhausted one list." << std::endl;
    while (i < size_a) {
        out[i+j] = a[i];
        i++;
    }
    while (j < size_b) {
        out[i+j] = b[j];
        j++;
    }
    if(DEBUGM) std::cerr << "Done binary merging." << std::endl;
    return out;
}

/// divide&conquer algorithm to recursively merge seq_n sequences, each of size seq_size
int * merge(int* sequences, int seq_size, int seq_n) {

    if(DEBUGM) std::cerr << "Merge - seq_size: " << seq_size << " - seq_n: " << seq_n << " - sequences ptr: " << sequences << std::endl;

    if(seq_n == 1) {
        if(DEBUGM) std::cerr << "Base case (one sequence)" << std::endl;
        return sequences;
    }
    if(seq_n == 2) {
        if(DEBUGM) std::cerr << "Base case (two sequences)" << std::endl;
        return binary_merge(sequences, seq_size, sequences + seq_size, seq_size);
    }

    if(DEBUGM) std::cerr << "Divide - offset: " << seq_n/2*seq_size << std::endl;
    int * out_a = merge(sequences, seq_size, seq_n/2);
    if(DEBUGM) std::cerr << "Divide - first merge done - out_a pointer: " << out_a << std::endl;
    if(DEBUGM) std::cout << "Result of first merge:" << std::endl;
    if(DEBUGM) listout(out_a, seq_size);
    int * out_b = merge(sequences + seq_n/2*seq_size, seq_size, seq_n/2);
    if(DEBUGM) std::cerr << "Divide - second merge done - out_b pointer: " << out_b << std::endl;
    if(DEBUGM) std::cout << "Result of second merge:" << std::endl;
    if(DEBUGM) listout(out_b, seq_size);

    if(DEBUGM) std::cerr << "Conquer" << std::endl;
    return binary_merge(out_a, seq_size*seq_n/2, out_b, seq_size*seq_n/2);
}