#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define _USE_MATH_DEFINES
#define strKota 25
#define r 6371
double minDist = 99999999999999999999.0;

struct Ants {
   int ant_index;
   int ant_path[strKota];
   char ant_city[strKota];
};

double degtoRad(double degree)
{
    return degree * 3.14 / 180.0;
}

double calcDistance(double lat1, double lat2, double long1, double long2)
{
    return 2 * r * asin(sqrt(pow(sin((lat1 - lat2) / 2), 2) + cos(lat1) * cos(lat2) * pow(sin((long1 - long2) / 2), 2)));
}

// Fungsi untuk menghitung panjang rute dari sebuah jalur
double calculate_distance(int* path, double** distances, size_t num_cities)
{
    double distance = 0.0;
    for (size_t i = 0; i < num_cities - 1; i++)
        distance += distances[path[i]][path[i + 1]];
    distance += distances[path[num_cities - 1]][path[0]]; // Kembali ke kota awal
    return distance;
}

// Fungsi untuk menginisialisasi feromon awal antarkota
void initialize_pheromones(double **pheromones, size_t num_cities)
{
    for (size_t i = 0; i < num_cities; i++)
    {
        for (size_t j = 0; j < num_cities; j++)
            pheromones[i][j] = 1.0;
    }    
}

// Fungsi untuk memilih kota selanjutnya berdasarkan aturan probabilitas
char* select_next_city(char** kota, char* current_city, char** visited_cities, double** pheromones, double** distances, size_t num_cities)
{
    double* probabilities = (double*)calloc(num_cities, sizeof(double));
    double sum = 0.0;
    char* next_city = NULL; // Perbaikan 1: Menggunakan char* untuk next_city

    for (size_t i = 0; i < num_cities; i++)
    {
        if (!visited_cities[i])
        {
            if (distances[i][i] != 0)
            {
                probabilities[i] = pheromones[i][i] / distances[i][i];
                sum += probabilities[i];
            }
        }
    }

    double random_value = ((double)rand() / RAND_MAX) * sum;
    double cumulative_probability = 0.0;
    for (size_t i = 0; i < num_cities; i++)
    {
        if (!visited_cities[i])
        {
            cumulative_probability += probabilities[i];
            if (cumulative_probability >= random_value)
            {
                next_city = strdup(kota[i]); // Perbaikan 2: Gunakan strdup untuk menyalin string
                break;
            }
        }
    }

    free(probabilities);
    return next_city; // Perbaikan 3: Mengembalikan alamat memori yang telah dialokasikan
}

// Fungsi untuk memperbarui jejak feromon setelah semua semut selesai menjelajah
void update_pheromones(double** pheromones, struct Ants* ant, double** distances, double evaporation_rate, size_t num_cities)
{
    for (size_t i = 0; i < num_cities; i++)
    {
        for (size_t j = 0; j < num_cities; j++)
            pheromones[i][j] *= (1.0 - evaporation_rate); // Penguapan feromon
    }
    for (size_t k = 0; k < num_cities; k++)
    {
        double distance = calculate_distance(ant[k].ant_path, distances, num_cities);
        for (size_t i = 0; i < num_cities - 1; i++)
        {
            pheromones[ant[k].ant_path[i]][ant[k].ant_path[i+1]] += (1.0 / distance); // Penambahan feromon
            pheromones[ant[k].ant_path[i+1]][ant[k].ant_path[i]] += (1.0 / distance); // Karena grafik tidak berarah
        }
        pheromones[ant[k].ant_path[num_cities - 1]][ant[k].ant_path[0]] += (1.0 / distance); // Kembali ke kota awal
        pheromones[ant[k].ant_path[0]][ant[k].ant_path[num_cities - 1]] += (1.0 / distance); // Karena grafik tidak berarah
    }
}

void ACO(char** kota, size_t num_cities, char* current_city, double** distances, int* path, double** pheromones, int max_iterations, double evaporation_rate)
{
    // Inisialisasi feromon awal
    initialize_pheromones(pheromones, num_cities);

    // Algoritma ACO
    char** visited_cities = (char**)calloc(num_cities, sizeof(char*));
    // Allocate memory for n Ants structs
    struct Ants* ants = (struct Ants*)calloc(num_cities, sizeof(struct Ants));
    // Check if memory allocation was successful
    if (ants == NULL || visited_cities == NULL) {
        printf("Error: Memory allocation failed.\n");
        return;
    }

    for (int iter = 0; iter < max_iterations; iter++)
    {
        // Semut menjelajah
        for (size_t k = 0; k < num_cities; k++)
        {
            ants[k].ant_index = k;
            strcpy(ants[k].ant_city, current_city);
            visited_cities[k] = current_city;
            for (size_t i = 1; i < num_cities; i++)
            {
                char* next_city = select_next_city(kota, current_city, visited_cities, pheromones, distances, num_cities);
                strcpy(ants[k].ant_city, next_city); // Perbaikan 1: Menyalin string ke ants[k].ant_city
                visited_cities[i] = next_city;
                current_city = next_city; // Perbaikan 2: Memperbarui current_city setelah menjelajahi semua kota
            }
        }

        // Memperbarui jejak feromon
        update_pheromones(pheromones, ants, distances, evaporation_rate, num_cities);

        // Memeriksa apakah rute saat ini lebih baik dari yang sebelumnya
        double current_distance = calculate_distance(ants[0].ant_path, distances, num_cities);
        if (current_distance < minDist)
        {
            minDist = current_distance;
            for (size_t i = 0; i < num_cities; i++)
            {
                path[i] = ants[i].ant_path[i]; // Menyimpan rute terbaik
            }
        }
    }

    free(visited_cities);
    free(ants);
}

int main()
{
    char** kota;
    double* latitude;
    double* longitude;

    char nama_file[12];
    printf("Nama file berisi kota untuk di kunjungi : ");
    scanf("%s", nama_file);
    FILE* file = fopen(nama_file, "r");

    // Mengambil bagian extension/format file user
    char* token;
    token = strtok(nama_file, ".");
    token = strtok(NULL, "\n");

    // Jika input bukan format file (Tanpa ".")
    if (token == NULL)
        printf("\nInput bukan file.\n");
    // Jika format bukan csv
    else if (strcmp(token, "csv") != 0)
    {
        printf("\nFormat file bukan csv.\n");
    }
    // Jika file tidak ada
    else if (file == NULL)
    {
        printf("\nFile yang diberikan tidak ada.\n");
    }
    else
    {
        // Input nama kota
        char nama_kota[strKota];
        printf("Starting point : ");
        getchar();
        fgets(nama_kota, sizeof(nama_kota), stdin);
        nama_kota[strcspn(nama_kota, "\n")] = 0; // Menghapus newline character


        char temp[50];
        // Deklarasi variabel untuk operasi ekstrak data csv
        int valid = 0;
        int max_index = 0;
        int start_index = 0;


        // Ekstrak data csv
        while (fgets(temp, 50, file))
        {
            // Alokasi memori untuk kota,latitude,longitude
            if (max_index == 0)
            {
                kota = (char **)malloc(sizeof(char *));
                kota[max_index] = (char *)malloc(strKota * sizeof(char));
                latitude = (double *)malloc(sizeof(double));
                longitude = (double *)malloc(sizeof(double));
            }
            else
            {
                kota = (char **)realloc(kota, (max_index + 1) * sizeof(char *));
                kota[max_index] = (char *)malloc(strKota * sizeof(char));
                latitude = (double *)realloc(latitude, (max_index + 1) * sizeof(double));
                longitude = (double *)realloc(longitude, (max_index + 1) * sizeof(double));
            }
        
            token = strtok(temp, ",");
            strcpy(kota[max_index], token);
            // Mengecek nama kota ada di list atau tidak
            if (strcmp(token, nama_kota) == 0)
            {
                valid = 1;
                start_index = max_index;
            }

            token = strtok(NULL, ",");
            latitude[max_index] = degtoRad(atof(token));
            token = strtok(NULL, "\n");
            longitude[max_index] = degtoRad(atof(token));
            max_index++;
        }

        fclose(file);
        // Data kosong
        if (max_index == 0)
        {
            printf("\nFile yang diberikan kosong.\n");
        }

        // Data hanya 1
        else if (max_index == 1)
        {
            printf("\nKota hanya satu, tidak bisa dioperasikan.\n");
        }

        // Jika kota diinput tidak ada
        else if (!valid)
        {
            printf("\nKota yang diinput tidak ada dalam file.\n");
        }

        else
        {
            // Waktu mulai mencari solusi
            clock_t start_time = clock();

            // Deklarasi Variabel untuk DFS
            int path[max_index];
            int** ants = (int**)malloc(max_index * sizeof(int*));
            for (int i = 0; i < max_index; i++)
            {
                ants[i] = (int*)malloc(max_index * sizeof(int));
            }
            double** distances = (double**)malloc(max_index * sizeof(double *));
            for (int i = 0; i < max_index; i++)
            {
                distances[i] = (double*)malloc(max_index * sizeof(double));
            }
            for (int i = 0; i < max_index; i++)
            {
                for (int j = 0; j < max_index; j++)
                {
                    if (i != j)
                    {
                        distances[i][j] = calcDistance(latitude[i], latitude[j], longitude[i], longitude[j]);
                    }
                    else
                    {
                        distances[i][j] = 0;
                    }
                }
            }
            

            // Rekursif DFS-Bruteforce
            int best_path[max_index];
            double** pheromones = (double**)malloc(max_index * sizeof(double *));
            for (int i = 0; i < max_index; i++)
            {
                pheromones[i] = (double*)malloc(max_index * sizeof(double));
            }

            ACO(kota, max_index, nama_kota, distances, best_path, pheromones, 100, 0.5);

            // Waktu selesai mencari solusi
            clock_t end_time = clock();

            // Output path
            printf("\nBest route :\n");
            for (int i = 0; i < max_index; i++)
            {
                printf("%s -> ", best_path[i]);
            }
            printf("%s\n", kota[start_index]);
            printf("\nTotal distance : %.4f km\n", minDist);

            // Menghitung waktu yang diperlukan
            double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
            printf("Time elapsed : %.3f s", execution_time);

            // Free memory
            for (int i = 0; i < max_index; i++)
            {
                free(ants[i]);
                free(pheromones[i]);
            }
            free(ants);
            free(pheromones);
        }
    }

    return 0;
}