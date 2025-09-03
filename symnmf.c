
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "symnmf.h"

double Euclidean_distance(struct cord *cord1, struct cord *cord2) {
    double distance = 0;
    while (cord1 != NULL && cord2 != NULL) {
        distance += (cord1->value - cord2->value) * (cord1->value - cord2->value);
        cord1 = cord1->next;
        cord2 = cord2->next;
    }
    return distance;
}

struct vector* ReadVectors(char* filename) {
    FILE *fp = fopen(filename, "r");
    assert(fp);
    struct vector *head_vec = NULL, *curr_vec = NULL, *new_vec = NULL;
    struct cord *head_cord = NULL, *curr_cord = NULL, *new_cord = NULL;
    double n;
    char c;
    int first = 1;
    while (fscanf(fp, "%lf%c", &n, &c) == 2) {
        new_cord = malloc(sizeof(struct cord));
        if (!new_cord) {
            perror("malloc failed");
            fclose(fp);
            return head_vec;
        }
        new_cord->value = n;
        new_cord->next = NULL;

        if (!head_cord) {
            head_cord = curr_cord = new_cord;
        } else {
            curr_cord->next = new_cord;
            curr_cord = new_cord;
        }

        if (c == '\n') {
            new_vec = malloc(sizeof(struct vector));
            if (!new_vec) {
                assert(new_vec);
                fclose(fp);
                return head_vec;
            }
            new_vec->cords = head_cord;
            new_vec->next = NULL;
            if (first) {
                head_vec = curr_vec = new_vec;
                first = 0;
            } else {
                curr_vec->next = new_vec;
                curr_vec = new_vec;
            }
            head_cord = curr_cord = NULL; 
        }
    }
    if (head_cord != NULL) {
        new_vec = malloc(sizeof(struct vector));
        new_vec->cords = head_cord;
        new_vec->next = NULL;
        if (first) {
            head_vec = new_vec;
        } else {
            curr_vec->next = new_vec;
        }
    }
    fclose(fp);
    return head_vec;
}

void print_vectors(struct vector *head, int k) {
    struct vector *curr_vec = head;
    while (curr_vec != NULL) {
        struct cord *curr_cord = curr_vec->cords;
        while (curr_cord != NULL) {
            printf("%.4f", curr_cord->value);
            if (curr_cord->next != NULL) {
                printf(",");
            }
            curr_cord = curr_cord->next;
        }
        printf("\n");

        curr_vec = curr_vec->next;
    }
}

int count_vectors(struct vector* head){
    int count =0;
    struct vector *curr = head;
    while (curr != NULL)
    {
        curr = curr -> next;
        count++;
    }
    return count;
}

double** sym(struct vector *head, int n) {
    double **A = calloc(n, sizeof(double*));
    assert(A);

    struct vector *curr_i = head;
    struct vector *curr_j;
    struct cord *xi, *xj;
    int i, j;
    double dist;

    for (i = 0; i < n; i++) {
        A[i] = calloc(n, sizeof(double));
        assert(A[i]);
    }

    for (i = 0; i < n; i++) {
        xi = curr_i->cords;
        curr_j = head;
        for (j = 0; j < n; j++) {
            xj = curr_j->cords;
            if (i != j) {
                dist = Euclidean_distance(xi, xj);
                dist = -dist / (2.0 );
                A[i][j] = exp(dist);
            } else {
                A[i][j] = 0.0;
            }
            curr_j = curr_j->next;
        }
        curr_i = curr_i->next;
    }
    return A;
}

double** ddg(double **A, int n){
    double** D = calloc(n,sizeof(double *));
    assert(D);
    int i,j;
    double sum;
    for (i = 0; i < n; i++) {
        D[i] = calloc(n, sizeof(double));
        assert(D[i]);
        sum = 0.0;
        for(j = 0; j < n; j++){
            sum = sum + A[i][j];
        }
        D[i][i] = sum; 
    }
    return D;
}

double** norm(double** A, double** D, int n){
    double** W = calloc(n,sizeof(double*));
    assert(W);
    int i,j;
    for (i = 0; i < n; i++) {
        W[i] = calloc(n, sizeof(double));
        assert(W[i]);
    }
    for(i = 0; i < n; i++){
        for(j = 0; j < n; j++){
            if(i != j){
                W[i][j] = A[i][j] / sqrt(D[i][i] * D[j][j]);
            }
        }
    }
    return W;
}

void PrintMatrix(double **matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%.4f ", matrix[i][j]);
        }
        printf("\n");
    }
}

void Free_Matrix(double **matrix, int n) {
    for (int i = 0; i < n; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void Free_vector(struct vector *head) {
    struct vector *curr_vec = head;
    while (curr_vec != NULL) {
        struct cord *curr_cord = curr_vec->cords;
        while (curr_cord != NULL) {
            struct cord *temp = curr_cord;
            curr_cord = curr_cord->next;
            free(temp);
        }
        struct vector *temp = curr_vec;
        curr_vec = curr_vec->next;
        free(temp);
    }
}

int str_eq(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) return 0;  
        a++;
        b++;
    }
    return *a == *b;  
}

int main(int argc, char *argv[]) {

    char *goal = argv[1];
    char *filename = argv[2];


    struct vector* curr = ReadVectors(filename);
    // print_vectors(curr, 10);

    int n = count_vectors(curr);
    if (str_eq(goal,"sym")){
        double **A = sym(curr, n);
        PrintMatrix(A,n);
        Free_Matrix(A,n);
    }

    if (str_eq(goal,"ddg")){
        double **A = sym(curr, n);
        double **D = ddg(A, n);
        PrintMatrix(D,n);
        Free_Matrix(A,n);
        Free_Matrix(D,n);
    }

    if (str_eq(goal,"norm")){
        double **A = sym(curr, n);
        double **D = ddg(A, n);
        double **W = norm(A, D, n);
        PrintMatrix(W,n);
        Free_Matrix(A,n);
        Free_Matrix(D,n);
        Free_Matrix(W,n);
    }
    
    Free_vector(curr);

    exit(0);
}
