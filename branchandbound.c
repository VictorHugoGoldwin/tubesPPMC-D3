#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>

#define strKota 25
#define r 6371
#define M_PI 3.14159265358979323846

double minDist = DBL_MAX;

// Fungsi untuk mengubah sudut dari derajat ke radian
double degtoRad(double degree) {
    return degree * M_PI / 180.0;
}

// Fungsi untuk menghitung jarak antara dua titik koordinat menggunakan rumus haversine
double calcDistance(double lat1, double lat2, double long1, double long2) {
    return 2 * r * asin(sqrt(pow(sin((lat1 - lat2) / 2), 2) + cos(lat1) * cos(lat2) * pow(sin((long1 - long2) / 2), 2)));
}

// Fungsi untuk mengisi matriks distance_cache dengan jarak antara setiap pasang kota
void fillDistanceCache(int max_index, double latitude[], double longitude[], double distance_cache[max_index][max_index]) {
    for (int i = 0; i < max_index; i++) {
        for (int j = 0; j < max_index; j++) {
            if (i != j) {
                // Hitung jarak antara dua titik menggunakan rumus haversine
                distance_cache[i][j] = calcDistance(latitude[i], latitude[j], longitude[i], longitude[j]);
            } else {
                // Jarak dari titik ke dirinya sendiri adalah 0
                distance_cache[i][j] = 0.0;
            }
        }
    }
}

// Fungsi untuk menghitung batas bawah (lower bound) untuk jarak yang tersisa dalam rute
double calcLowerBound(int max_index, int visited[], int current, double distance_cache[max_index][max_index], int start_index) {
    double lower_bound = 0;
    int unvisited_count = 0;
    double min_dist_to_unvisited = DBL_MAX;

    // Iterasi semua kota untuk mencari yang belum dikunjungi dan jarak minimum ke kota yang belum dikunjungi
    for (int i = 0; i < max_index; i++) {
        if (!visited[i]) {
            unvisited_count++;
            double dist = distance_cache[current][i];
            if (dist < min_dist_to_unvisited) {
                min_dist_to_unvisited = dist;
            }
        }
    }

    // Jika hanya ada satu kota yang belum dikunjungi, jarak minimum adalah kembali ke titik awal
    if (unvisited_count == 1) {
        min_dist_to_unvisited = distance_cache[current][start_index];
    }

    // Hitung lower bound
    lower_bound = min_dist_to_unvisited;
    if (unvisited_count > 1) {
        lower_bound += (unvisited_count - 1) * min_dist_to_unvisited;
    }

    return lower_bound;
}

// Fungsi rekursif untuk mencari rute terpendek menggunakan algoritma Branch and Bound
void BranchAndBound(int max_index, int start_index, int current_City, int path_index, int path[], int short_path[], int visited[], double distance_cache[max_index][max_index], double distance) {
    visited[current_City] = 1;
    path[path_index] = current_City;

    // Jika semua kota telah dikunjungi
    if (path_index == max_index - 1) {
        distance = distance + distance_cache[current_City][start_index];

        // Periksa apakah rute yang baru ditemukan lebih pendek dari yang sebelumnya
        if (distance < minDist) {
            minDist = distance;
            // Salin rute terpendek ke short_path
            for (int i = 0; i < max_index; i++) {
                short_path[i] = path[i];
            }
        }
    } else {
        // Inisialisasi array untuk menyimpan lower bound dari setiap kota yang belum dikunjungi
        double lower_bounds[max_index];
        // Inisialisasi array untuk menyimpan nomor indeks kota yang belum dikunjungi
        int cities[max_index];
        // Inisialisasi array untuk menyimpan nomor kota yang sudah diurutkan berdasarkan lower bound
        int sorted_index[max_index];
        int sorted_count = 0;

        // Hitung lower bound untuk setiap kota yang belum dikunjungi
        for (int i = 0; i < max_index; i++) {
            if (!visited[i]) {
                lower_bounds[i] = calcLowerBound(max_index, visited, i, distance_cache, start_index);
                cities[i] = i;
                sorted_index[sorted_count++] = i;
            }
        }

        // Urutkan kota berdasarkan lower bound
        for (int i = 0; i < sorted_count - 1; i++) {
            for (int j = 0; j < sorted_count - i - 1; j++) {
                if (lower_bounds[sorted_index[j]] > lower_bounds[sorted_index[j + 1]]) {
                    int temp = sorted_index[j];
                    sorted_index[j] = sorted_index[j + 1];
                    sorted_index[j + 1] = temp;
                }
            }
        }

        // Lakukan pencarian rekursif untuk setiap kota yang belum dikunjungi
        for (int i = 0; i < sorted_count; i++) {
            int nextCity = sorted_index[i];
            double next_dist = distance_cache[current_City][nextCity];
            double new_distance = distance + next_dist;

            // Jika jarak baru lebih pendek dari jarak terpendek yang ada, lanjutkan pencarian rekursif
            if (new_distance < minDist) {
                BranchAndBound(max_index, start_index, nextCity, path_index + 1, path, short_path, visited, distance_cache, new_distance);
            }
        }
    }

    // Set kota yang sedang dikunjungi menjadi belum dikunjungi lagi
    visited[current_City] = 0;
}

int main() {
    // Deklarasi variabel
    char **kota; // Array untuk menyimpan nama kota
    double *latitude; // Array untuk menyimpan nilai latitude kota
    double *longitude; // Array untuk menyimpan nilai longitude kota

    char nama_file[12]; // Variabel untuk menyimpan nama file yang akan dibaca
    // Meminta nama file yang berisi daftar kota untuk dikunjungi
    printf("Nama file berisi kota untuk di kunjungi : ");
    scanf("%s", nama_file);
    FILE *file = fopen(nama_file, "r"); // Membuka file dalam mode baca ("r")

    char *token;
    token = strtok(nama_file, ".");
    token = strtok(NULL, "\n");

    // Memeriksa format nama file
    if (token == NULL)
        printf("\nInput bukan file.\n");
    else if (strcmp(token, "csv") != 0) {
        printf("\nFormat file bukan csv.\n");
    } else if (file == NULL) { // Memeriksa keberadaan file
        printf("\nFile yang diberikan tidak ada.\n");
    } else {
        char nama_kota[strKota];
        // Meminta nama kota sebagai titik awal perjalanan
        printf("Starting point : ");
        getchar(); // Membersihkan buffer stdin
        scanf("%[^\n]s", nama_kota);

        char temp[50];
        int valid = 0;
        int max_index = 0;
        int start_index = 0;

        int initial_capacity = 10; // Kapasitas awal array
        // Mengalokasikan memori untuk array kota, latitude, dan longitude
        kota = (char **)malloc(initial_capacity * sizeof(char *));
        latitude = (double *)malloc(initial_capacity * sizeof(double));
        longitude = (double *)malloc(initial_capacity * sizeof(double));

        // Membaca file dan memproses informasi kota
        while (fgets(temp, 50, file)) {
            // Melakukan realokasi memori jika kapasitas awal terlampaui
            if (max_index >= initial_capacity) {
                initial_capacity *= 2;
                // Melakukan realokasi memori untuk array kota, latitude, dan longitude
                kota = (char **)realloc(kota, initial_capacity * sizeof(char *));
                latitude = (double *)realloc(latitude, initial_capacity * sizeof(double));
                longitude = (double *)realloc(longitude, initial_capacity * sizeof(double));
            }

            kota[max_index] = (char *)malloc(strKota * sizeof(char));

            token = strtok(temp, ",");
            // Menyalin nama kota
            strcpy(kota[max_index], token);
            // Memeriksa apakah kota merupakan titik awal perjalanan
            if (strcmp(token, nama_kota) == 0) {
                valid = 1;
                start_index = max_index;
            }

            // Membaca dan mengonversi latitude dan longitude ke radian
            token = strtok(NULL, ",");
            latitude[max_index] = degtoRad(atof(token));
            token = strtok(NULL, "\n");
            longitude[max_index] = degtoRad(atof(token));
            max_index++;
        }

        fclose(file); // Menutup file setelah selesai membaca
        // Memeriksa kondisi setelah membaca file
        if (max_index == 0) {
            printf("\nFile yang diberikan kosong.\n");
        } else if (max_index == 1) {
            printf("\nKota hanya satu, tidak bisa dioperasikan.\n");
        } else if (!valid) {
            printf("\nKota yang diinput tidak ada dalam file.\n");
        } else {
            clock_t start_time = clock(); // Waktu mulai eksekusi program

            int path[max_index]; // Array untuk menyimpan rute sementara
            int visited[max_index]; // Array untuk menyimpan status kunjungan kota
            int short_path[max_index]; // Array untuk menyimpan rute terpendek
            double distance_cache[max_index][max_index]; // Matriks untuk menyimpan jarak antara kota
            // Mengisi matriks distance_cache dengan jarak antara setiap pasang kota
            fillDistanceCache(max_index, latitude, longitude, distance_cache);

            // Inisialisasi status kunjungan kota
            for (int i = 0; i < max_index; i++) {
                visited[i] = 0;
            }

            // Memulai pencarian rute terpendek menggunakan algoritma Branch and Bound
            BranchAndBound(max_index, start_index, start_index, 0, path, short_path, visited, distance_cache, 0.0);

            clock_t end_time = clock(); // Waktu selesai eksekusi program

            // Menampilkan hasil rute terpendek
            printf("\nBest route :\n");
            for (int i = 0; i < max_index; i++) {
                printf("%s -> ", kota[short_path[i]]);
            }
            printf("%s\n", kota[start_index]);
            printf("\nTotal distance : %.4f km\n", minDist); // Menampilkan total jarak terpendek

            // Menghitung dan menampilkan waktu eksekusi program
            double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
            printf("Time elapsed : %.3f s\n", execution_time);
        }

        // Membebaskan memori yang dialokasikan untuk array kota, latitude, dan longitude
        for (int i = 0; i < max_index; i++) {
            free(kota[i]);
        }
        free(kota);
        free(latitude);
        free(longitude);
    }

    return 0;
}
