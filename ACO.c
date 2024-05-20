/*EL2208 Praktikum Pemecahan Masalah dengan C 2023/2024
*Modul            : 9 - Tugas Besar
*Hari dan Tanggal : 
*Nama (NIM)       : Rafi Ananta Alden (13222087)
*Asisten (NIM)    : Isnaini Azhar Ramadhan Wijaya (18321016)
*Nama File        : ACO.c
*Deskripsi        : Program untuk menyelesaikan Travelling Salesman Problem (TSP) dengan menggunakan algoritma Ant Colony Optimization (ACO)
*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define bool _Bool
#define strKota 25
#define r 6371
#define M_PI 3.14159265358979323846
double minDist = 99999999999999999999.0;

#define ALPHA 1.0  // Pheromone importance
#define BETA 2.0   // Heuristic importance
#define RHO 0.5    // Evaporation rate
#define Q 100      // Pheromone deposit factor
#define ANT_COUNT 10
#define MAX_ITERATIONS 100

double degtoRad(double degree)
{
    return degree * M_PI / 180.0;
}

double calcDistance(double lat1, double lat2, double long1, double long2)
{
    return 2*r*asin(sqrt(pow(sin((lat1-lat2)/2),2) + cos(lat1)*cos(lat2)*pow(sin((long1-long2)/2),2)));
}

void initialize_pheromones(int num_cities, double pheromones[num_cities][num_cities]) {
    for (int i = 0; i < num_cities; i++) {
        for (int j = 0; j < num_cities; j++) {
            pheromones[i][j] = 0.1;
        }
    }
}

void update_pheromones(int num_cities, double pheromones[num_cities][num_cities], int ant_tours[][num_cities], double tour_distances[]) {
    for (int i = 0; i < num_cities; i++) {
        for (int j = 0; j < num_cities; j++) {
            pheromones[i][j] *= (1.0 - RHO);
        }
    }
    for (int k = 0; k < ANT_COUNT; k++) {
        for (int i = 0; i < num_cities; i++) {
            int current_city = ant_tours[k][i];
            int next_city = ant_tours[k][(i + 1) % num_cities];
            pheromones[current_city][next_city] += Q / tour_distances[k];
            pheromones[next_city][current_city] += Q / tour_distances[k];
        }
    }
}

void calculate_probabilities(int num_cities, double pheromones[num_cities][num_cities], double distances[num_cities][num_cities], double probabilities[num_cities][num_cities]) {
    for (int i = 0; i < num_cities; i++) {
        for (int j = 0; j < num_cities; j++) {
            if (i != j) {
                double pheromone = pow(pheromones[i][j], ALPHA);
                double distance = pow(1.0 / distances[i][j], BETA);
                probabilities[i][j] = pheromone * distance;
            } else {
                probabilities[i][j] = 0.0;
            }
        }
    }
}

int select_next_city(int num_cities, double probabilities[num_cities][num_cities], int ant_id, bool visited[]) {
    double sum = 0.0;
    for (int i = 0; i < num_cities; i++) {
        if (!visited[i]) {
            sum += probabilities[ant_id][i];
        }
    }
    double rand_num = (double)rand() / RAND_MAX;
    double partial_sum = 0.0;
    for (int i = 0; i < num_cities; i++) {
        if (!visited[i]) {
            partial_sum += probabilities[ant_id][i];
            if (rand_num <= partial_sum / sum) {
                return i;
            }
        }
    }
    return -1;
}

double ant_tour(int num_cities, int starting_point, double distances[num_cities][num_cities], double probabilities[num_cities][num_cities], int ant_id, int tour[]) {
    bool visited[num_cities];
    for (int i = 0; i < num_cities; i++) {
        visited[i] = false;
    }
    visited[starting_point] = true;
    tour[0] = starting_point;
    double tour_distance = 0.0;
    for (int i = 1; i < num_cities; i++) {
        int next_city = select_next_city(num_cities, probabilities, ant_id, visited);
        tour[i] = next_city;
        visited[next_city] = true;
        tour_distance += distances[starting_point][next_city];
        starting_point = next_city;
    }
    tour_distance += distances[starting_point][0]; // Return to starting city
    return tour_distance;
}

void print_best_tour(int num_cities, char** cities, char starting_point[strKota], int best_tour[]) {
    printf("Best tour: ");
    for (int i = 0; i < num_cities; i++) {
        printf("%s -> ", cities[best_tour[i]]);
    }
    printf("%s\n", starting_point);
}

int main()
{
    char** kota;
    double* latitude;
    double* longitude;
    double* distance;

    char nama_file[12];
    printf("Nama file berisi kota untuk di kunjungi : ");
    scanf("%s",nama_file);
    FILE* file = fopen(nama_file,"r");

    // Mengambil bagian extension/format file user
    char* token;
    token = strtok(nama_file,".");
    token = strtok(NULL,"\n");
    
    // Jika input bukan format file (Tanpa ".")
    if (token == NULL) printf("\nInput bukan file.\n");
    // Jika format bukan csv
    else if (strcmp(token,"csv") != 0) {printf("\nFormat file bukan csv.\n");}
    // Jika file tidak ada
    else if (file == NULL) {printf("\nFile yang diberikan tidak ada.\n");}
    else {
        // Input nama kota
        char nama_kota[strKota];
        printf("Starting point : ");
        getchar();
        scanf("%[^\n]s",nama_kota);

        char temp[50];
        // Deklarasi variabel untuk operasi ekstrak data csv
        int valid = 0;int max_index = 0;int start_index = 0;

        // Ekstrak data csv
        while(fgets(temp,50,file)) {
            // Alokasi memori untuk kota,latitude,longitude
            if (max_index == 0) {
                kota = (char**) malloc (sizeof(char*));
                kota[max_index] = (char*) malloc (strKota * sizeof(char));
                latitude = (double*) malloc (sizeof(double));
                longitude = (double*) malloc (sizeof(double));
            }        
            else {
                kota = (char**) realloc (kota, (max_index+1)*sizeof(char*));
                kota[max_index] = (char*) malloc (strKota * sizeof(char));
                latitude = (double*) realloc (latitude, (max_index+1)*sizeof(double));
                longitude = (double*) realloc (longitude, (max_index+1)*sizeof(double));
            }

            token = strtok(temp,",");
            strcpy(kota[max_index],token);
            // Mengecek nama kota ada di list atau tidak
            if (strcmp(token,nama_kota) == 0) {
                valid = 1;
                start_index = max_index;
            }

            token = strtok(NULL,",");
            latitude[max_index] = degtoRad(atof(token));
            token = strtok(NULL,"\n");
            longitude[max_index] = degtoRad(atof(token));
            max_index++;
        }

        fclose(file);
        // Data kosong
        if (max_index == 0) {printf("\nFile yang diberikan kosong.\n");}

        // Data hanya 1
        else if (max_index == 1) {printf("\nKota hanya satu, tidak bisa dioperasikan.\n");}
        
        // Jika kota diinput tidak ada
        else if (!valid) {printf("\nKota yang diinput tidak ada dalam file.\n");}

        else {
            // Waktu mulai mencari solusi
            clock_t start_time = clock();

            // Membuat matriks jarak antarkota
            double distances[max_index][max_index];
            for (int i = 0; i < max_index; i++) {
                for (int j = 0; j < max_index; j++) {
                    if (i != j)
                        distances[i][j] = calcDistance(latitude[i], latitude[j], longitude[i], longitude[j]);
                    else
                        distances[i][j] = 0;
                }
            }

            double pheromones[max_index][max_index];
            double probabilities[max_index][max_index];

            initialize_pheromones(max_index, pheromones);
            calculate_probabilities(max_index, pheromones, distances, probabilities); // Perubahan #1

            int best_tour[max_index];
            double best_distance = -1;

            for (int iteration = 0; iteration < MAX_ITERATIONS; iteration++) {
                int ant_tours[ANT_COUNT][max_index];
                double tour_distances[ANT_COUNT];
                for (int ant_id = 0; ant_id < ANT_COUNT; ant_id++) {
                    tour_distances[ant_id] = ant_tour(max_index, start_index, distances, probabilities, ant_id, ant_tours[ant_id]);
                    if (best_distance == -1 || tour_distances[ant_id] < best_distance) {
                        best_distance = tour_distances[ant_id];
                        for (int i = 0; i < max_index; i++) {
                            best_tour[i] = ant_tours[ant_id][i];
                        }
                    }
                }
                update_pheromones(max_index, pheromones, ant_tours, tour_distances);
                calculate_probabilities(max_index, pheromones, distances, probabilities);
            }
            
            // Waktu selesai mencari solusi
            clock_t end_time = clock();

            // Output path
            print_best_tour(max_index, kota, nama_kota, best_tour);

            printf("%s\n",kota[start_index]);
            printf("\nTotal distance : %.4f km\n",best_distance);

            // Menghitung waktu yang diperlukan
            double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
            printf("Time elapsed : %.3f s",execution_time);
        }
    }
}