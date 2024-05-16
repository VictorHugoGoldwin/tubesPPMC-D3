#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <time.h>


#define radius 6371
#define M_PI 3.14159265358979323846
typedef struct {
    char name[50];
    double x; // lattiude
    double y; // longitude
} City;

double distance(City a, City b) {
    double lat1 = a.x * M_PI / 180.0;
    double lon1 = a.y * M_PI / 180.0;
    double lat2 = b.x * M_PI / 180.0;
    double lon2 = b.y * M_PI / 180.0;

    double dlat = sin((lat2 - lat1) / 2) * sin((lat2 - lat1) / 2);
    double dlong = sin((lon2 - lon1) / 2) * sin((lon2 - lon1) / 2);
    return (2 * radius * asin(sqrt(dlat + cos(lat1) * cos(lat2) * dlong)));
}


void swap(int* path, int i, int j) {
    int temp = path[i];
    path[i] = path[j];
    path[j] = temp;
}
void permute(City* cities, int* path, int* minPath, double* minDist, int l, int r) {
    if (l == r) {
        double dist = 0;
        for (int i = 0; i < r; i++) {
            dist += distance(cities[path[i]], cities[path[i+1]]);
        }
        if (dist < *minDist) {
            *minDist = dist;
            for (int i = 0; i <= r; i++) {
                minPath[i] = path[i];
            }
        }
    } else {
        for (int i = l; i <= r; i++) {
            swap(path, l, i);
            permute(cities, path, minPath, minDist, l+1, r);
            swap(path, l, i);
        }
    }
}

int main() {
    char filename[50];
    printf("Enter the filename: ");
    fgets(filename, sizeof(filename), stdin);
    // remove newline character if present
    size_t len = strlen(filename);
    if (len > 0 && filename[len - 1] == '\n') {
    filename[len - 1] = '\0';
    }
    FILE* file = fopen(filename, "r");

    if (!file) {
        printf("File tidak ada mzzz\n");
        return 1;
    }

    City cities[50]; // Adjust size as needed
    int index = 0;

    char line[1024];
    while (fgets(line, 1024, file)) {
        char* tmp = strdup(line);
        char* tok = strtok(tmp, ",");

        strcpy(cities[index].name, tok);

        tok = strtok(NULL, ",");
        cities[index].x = atof(tok);

        tok = strtok(NULL, ",");
        cities[index].y = atof(tok);

        free(tmp);
        index++;
    }

    fclose(file);
    if (index == 0) {
        printf("File kosong");
        return 1;
    }

    char start[50];
    printf("Enter the starting city name: ");
    fgets(start, sizeof(start), stdin);
    // remove newline character if present
    size_t len2 = strlen(start);
    if (len2 > 0 && start[len2 - 1] == '\n') {
    start[len2 - 1] = '\0';
    }
    int n = index;
    int startIndex = -1;
    // start waktu
    clock_t start_time = clock();
    for (int i = 0; i < n; i++) {
        if (strcmp(cities[i].name, start) == 0) {
            startIndex = i;
            break;
        }
    }

    if (startIndex == -1) {
        printf("Starting city not found.\n");
        return 1;
    }

    int path[n];
    for (int i = 0; i < n; i++) path[i] = i;
    swap(path, 0, startIndex);

    int minPath[n];
    double minDist = DBL_MAX;

    permute(cities, path, minPath, &minDist, 1, n-1);
    // elapsed time calc
    clock_t end_time = clock();
    double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("Shortest path: %s", cities[minPath[0]].name);
    for (int i = 1; i < n; i++) {
        printf(" -> %s", cities[minPath[i]].name);
    }
    printf(" -> %s", start);
    // add the distance from the last city back to the starting city
    printf("\nTotal distance: %.2f\n", minDist + distance(cities[minPath[n-1]], cities[startIndex]));
    printf("Time elapsed : %.3f s",execution_time);
    return 0;
}

