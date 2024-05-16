#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#define strKota 25
#define r 6371
#define POPULATION_SIZE 10000
#define MUTATION_RATE 0.1
#define MAX_GENERATIONS 100
#define M_PI 3.14159265358979323846

typedef struct {
    int* path;
    double distance;
} Route;

//Fungsi untuk mengonversi derajat ke radian
double degtoRad(double degree) {
    return degree * M_PI / 180.0;
}
//Fungsi untuk menghitung jarak antara dua kota
double calcDistance(double lat1, double lat2, double long1, double long2) {
    return 2 * r * asin(sqrt(pow(sin((lat1 - lat2) / 2), 2) + cos(lat1) * cos(lat2) * pow(sin((long1 - long2) / 2), 2)));
}
//Fungsi untuk mencari indeks kota awal (starting point)
int findStartingCityIndex(char starting_city[], char** kota, int max_index) {
    for (int i = 0; i < max_index; i++) {
        if (strcmp(starting_city, kota[i]) == 0) {
            return i;
        }
    }
    return -1; // Starting city tidak ditemukan
}
//Fungsi untuk membuat rute acak dari indeks kota
Route* generateRandomRoute(int NUM_CITIES, int starting_index) {
    Route* route = (Route*)malloc(sizeof(Route));
    route->path = (int*)malloc(NUM_CITIES * sizeof(int));
    bool* visited = (bool*)malloc(NUM_CITIES * sizeof(bool));

    for (int i = 0; i < NUM_CITIES; ++i) {
        route->path[i] = -1; // Inisialisasi dengan nilai yang tidak valid
        visited[i] = false; // Tandai semua kota sebagai belum dikunjungi
    }
    // Set kota awal sesuai dengan starting_index
    route->path[0] = starting_index;
    visited[starting_index] = true;
    for (int i = 1; i < NUM_CITIES; ++i) {
        int j;
        do {
            j = rand() % NUM_CITIES; // Pilih secara acak kota
        } while (visited[j]); // Ulangi jika kota telah dikunjungi sebelumnya
        route->path[i] = j; // Tandai kota sebagai dikunjungi
        visited[j] = true;
    }
    free(visited);
    return route;
}

//Fungsi untuk menghituk jarak setiap rute
double calculateRouteDistance(const Route* route, const double* latitude, const double* longitude, int index, int NUM_CITIES) {
    if (index >= NUM_CITIES - 1) {
        // Menambah jarak dari kota terakhir ke kota awal
        int lastCityIndex = route->path[NUM_CITIES - 1];
        int firstCityIndex = route->path[0];
        return calcDistance(latitude[lastCityIndex], latitude[firstCityIndex], longitude[lastCityIndex], longitude[firstCityIndex]);
    }
    int cityIndex1 = route->path[index];
    int cityIndex2 = route->path[index + 1];
    double distance = calcDistance(latitude[cityIndex1], latitude[cityIndex2], longitude[cityIndex1], longitude[cityIndex2]);
    return distance + calculateRouteDistance(route, latitude, longitude, index + 1, NUM_CITIES);
}

// Fungsi rekursif untuk menghitung total jarak rute
void calculateFitness(Route* route, int NUM_CITIES, double latitude[], double longitude[]) {
    route->distance = calculateRouteDistance(route, latitude, longitude, 0, NUM_CITIES);
}
//Fungsi untuk memeriksa apakah kota berada di dalam rute atau tidak
int isCityInPath(const Route* route, int city, int NUM_CITIES) {
    for (int i = 0; i < NUM_CITIES; ++i) {
        if (route->path[i] == city) {
            return 1;
        }
    }
    return 0;
}
//Fungsi untuk membuat rute anak dengan menggabungkan bagian dari dua rute induk
Route* crossover(const Route* parent1, const Route* parent2, int NUM_CITIES) {
    Route* child = (Route*)malloc(sizeof(Route));
    child->path = (int*)malloc(NUM_CITIES * sizeof(int));
    for (int i = 0; i < NUM_CITIES; ++i) {
        child->path[i] = -1;
    }
    int startPos = rand() % NUM_CITIES;
    int endPos = rand() % NUM_CITIES;

    if (startPos > endPos) {
        int temp = startPos;
        startPos = endPos;
        endPos = temp;
    }

    for (int i = startPos; i <= endPos; ++i) {
        child->path[i] = parent1->path[i];
    }
    int currentPos = 0;
    for (int i = 0; i < NUM_CITIES; ++i) {
        if (!isCityInPath(child, parent2->path[i], NUM_CITIES)) {
            while (child->path[currentPos] != -1) {
                currentPos++;
            }
            child->path[currentPos] = parent2->path[i];
        }
    }
    return child;
}

//Fungsi untuk secara acak menukar kota dalam rute dengan probabilitas yang ditentukan oleh MUTATION_RATE
void mutate(Route* route, int NUM_CITIES) {
    for (int i = 1; i < NUM_CITIES; ++i) {
        if ((double)rand() / RAND_MAX < MUTATION_RATE) {
            int j = 1 + rand() % (NUM_CITIES - 1);
            int temp = route->path[i];
            route->path[i] = route->path[j];
            route->path[j] = temp;
        }
    }
}

//Fungsi untuk mencari rute terbaik (rute terpendek) dari semua pupulasi
Route* findBestRoute(Route** population) {
    double shortestDistances = population[0]->distance;
    int bestIndex = 0;
    for (int i = 0; i < POPULATION_SIZE; ++i) {
        if (population[i]->distance < shortestDistances) { 
            shortestDistances = population[i]->distance;
            bestIndex = i;
        }
    }
    return population[bestIndex]; 
}
//Fungsi TSP menggunakan Algoritma Genetic
void tsp(double latitude[], double longitude[], int max_index, int starting_index, char **kota){
    Route* population[POPULATION_SIZE]; //Digunakan untuk menyimpan rute-rute
    for (int i = 0; i < POPULATION_SIZE; ++i) {
            population[i] = generateRandomRoute(max_index, starting_index); //Mengisi populasi dengan rute acak
            calculateFitness(population[i], max_index, latitude, longitude); //Menghitung jarak dari setiap rute
        }
    //Proses untuk menemukan rute terbaik menggunakan algoritma genetik
    for (int generation = 0; generation < MAX_GENERATIONS; ++generation) {
        Route* newPopulation[POPULATION_SIZE]; //Digunkan untuk generasi baru
        for (int i = 0; i < POPULATION_SIZE; ++i) {
            Route* parent1 = findBestRoute(population);//Memilih dua rute terbaik dari populasi saat ini
            Route* parent2 = findBestRoute(population);
            Route* child = crossover(parent1, parent2, max_index);//Dilakukan crossover untuk dua rute terbaik untuk menghasilkan rute baru (child)
            mutate(child, max_index);//Melakukan mutasi pada rute (child)
            calculateFitness(child, max_index, latitude, longitude);//menghitung jarak dari rute (child)
            newPopulation[i] = child; //Rute (child) disimpan dalam populasi baru
        }
        //Menyalin newpopulation ke population untuk generasi berikutnya
        memcpy(population, newPopulation, POPULATION_SIZE * sizeof(Route*));
        }
    //Menentukan rute terbaik dari populasi
    Route* bestRoute = findBestRoute(population);
    // Print best route dan jarak
    for (int i = 0; i < max_index; i++) {
        printf("%s -> ", kota[bestRoute->path[i]]);
        }
        printf("%s\n", kota[bestRoute->path[0]]);
        printf("Jarak terdekat: %.5f km\n", bestRoute->distance);
    for (int i = 0; i < POPULATION_SIZE; ++i) {
        free(population[i]->path);
        free(population[i]);
        }

}

int main() {
    srand(time(NULL));
    char** kota;
    double* latitude;
    double* longitude;

    char nama_file[50];
    printf("Nama file berisi kota untuk di kunjungi : ");
    scanf("%s", nama_file);
    FILE* file = fopen(nama_file, "r");

    // Mengambil bagian extension/format file user
    char* token;
    token = strtok(nama_file, ".");
    token = strtok(NULL, "\n");
    
    // Jika input bukan format file (Tidak ada ".")
    if (token == NULL) {
        printf("\nInput bukan file.\n");
        return 1;
    }
    // Jika format bukan csv
    else if (strcmp(token, "csv") != 0) {
        printf("\nFormat file bukan csv.\n");
        return 1;
    }
    // Jika file tidak ada
    else if (file == NULL) {
        printf("\nFile yang diberikan tidak ada.\n");
        return 1;
    } else {
        // Input nama kota
        char nama_kota[strKota];
        printf("Starting point : ");
        getchar();
        scanf("%[^\n]s",nama_kota);

        char temp[50];
        // Deklarasi variabel untuk operasi ekstrak data csv
        int valid = 0 ; int max_index = 0;
        // Ekstrak data csv
        while (fgets(temp, 50, file)) {
            // Alokasi memori untuk kota, latitude, longitude
            if (max_index == 0) {
                kota = (char**)malloc(sizeof(char*));
                kota[max_index] = (char*)malloc(strKota * sizeof(char));
                latitude = (double*)malloc(sizeof(double));
                longitude = (double*)malloc(sizeof(double));
            } else {
                kota = (char**)realloc(kota, (max_index + 1) * sizeof(char*));
                kota[max_index] = (char*)malloc(strKota * sizeof(char));
                latitude = (double*)realloc(latitude, (max_index + 1) * sizeof(double));
                longitude = (double*)realloc(longitude, (max_index + 1) * sizeof(double));
            }

            token = strtok(temp, ",");
            strcpy(kota[max_index], token);
            // Mengecek nama kota ada di list atau tidak
            if (strcmp(token, nama_kota) == 0) {
                valid = 1;
            }

            token = strtok(NULL, ",");
            latitude[max_index] = degtoRad(atof(token));
            token = strtok(NULL, "\n");
            longitude[max_index] = degtoRad(atof(token));
            max_index++;
        }
        // Data kosong
        if (max_index == 0) {
            printf("\nFile yang diberikan kosong.\n");
            return 1;
        }
        // Data hanya 1
        else if (max_index == 1) {
            printf("\nKota hanya satu, tidak bisa dioperasikan.\n");
            return 1;
        }
        // Jika kota diinput tidak ada
        else if (!valid) {
            printf("\nKota yang diinput tidak ada dalam file.\n");
            return 1;
        }
        // Jika kota diinput ada (Nanti ini diganti program TSP sesuai algoritma masing-masing)
        else {
            // Waktu mulai mencari solusi
            clock_t start_time = clock();
            printf("\nBest route: \n");
            int starting_index = findStartingCityIndex(nama_kota, kota, max_index);
            // Proses TSP Algoritma Genetic
            tsp(latitude, longitude, max_index, starting_index, kota);
            // Waktu selesai mencari solusi
            clock_t end_time = clock();
            // Menghitung waktu yang diperlukan
            double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
            printf("Time elapsed: %.5f s\n", execution_time);
            fclose(file);
            for (int i = 0; i < max_index; ++i) {
                free(kota[i]);
            }
            free(kota);
            free(latitude);
            free(longitude);
        }
    }
    return 0;
}
