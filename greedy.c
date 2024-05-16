/*EL2208 Praktikum Pemecahan Masalah dengan C 2023/2024
*Modul            : 9 - Tugas Besar
*Hari dan Tanggal : Jumat, 10 Mei 2024
*Nama (NIM)       : Filbert Garciano (13222086)
*Asisten (NIM)    : Isnaini Azhar Ramadhan Wijaya (18321016)
*Nama File        : greedy.c
*Deskripsi        : Membuat program menyelesaikan Travelling Salesman Problem dengan Greedy Algorithm
*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define strKota 25
#define r 6371
double minDist = 99999999999999999999.0;

double degtoRad(double degree)
{
    return degree * M_PI / 180.0;
}

double calcDistance(double lat1, double lat2, double long1, double long2)
{
    return 2*r*asin(sqrt(pow(sin((lat1-lat2)/2),2) + cos(lat1)*cos(lat2)*pow(sin((long1-long2)/2),2)));
}

void greedy(int max_index, int start, int current, int count, int path[], int short_path[], int visited[], double latitude[], 
double longitude[], double dist, char** kota) {
    visited[current] = 1;
    path[count] = current;
    // Base case: semua kota sudah di-visit
    if (count == max_index-1) {
        // Tambah jarak dari kota terakhir ke kota asal
        dist += calcDistance(latitude[current], latitude[start], longitude[current], longitude[start]);
        if (dist < minDist) {
            minDist = dist; // membuat jarak sekarang menjadi jarak terpendek
            for (int i = 0; i < max_index; i++) {
                short_path[i] = path[i];
            }
        }
        return;
    }

    // iterasi ke semua kota yang belum di visit
    double min_distance = INFINITY;
    int next_city = -1;
    for (int next = 0; next < max_index; next++) {
        if (!visited[next] && next != current) { // Tidak visit kota yang sudah dilalui
            // Hitung jarak
            double d = calcDistance(latitude[current], latitude[next], longitude[current], longitude[next]);
            if (d < min_distance) {
                min_distance = d;
                next_city = next;
            }
        }
    }
    if (next_city != -1) {
        // Hitung jarak dari kota saat ini ke kota selanjutnya
        dist += min_distance;
        // Lakukan rekursif ke kota selanjutnya
        greedy(max_index, start, next_city, count + 1, path, short_path, visited, latitude, longitude, dist, kota);
    }
}

int main() {
    char** kota;
    double* latitude;
    double* longitude;
    double* distance;

    char filename[12];
    printf("Nama file berisi kota untuk di kunjungi : ");
    scanf("%s", filename);

    // buka file
    FILE* file = fopen(filename, "r");

    // Mengambil bagian extension/format file user
    char* token;
    token = strtok(filename,".");
    token = strtok(NULL,"\n");
    
    // Jika input bukan format file (Tanpa ".")
    if (token == NULL) {
        printf("\nInput bukan file.\n");
        return 1;
    }
    // Jika format bukan csv
    else if (strcmp(token,"csv") != 0) {
        printf("\nFormat file bukan csv.\n");
        return 1;
    }
    // Jika file tidak ada
    else if (file == NULL) {
        printf("\nFile yang diberikan tidak ada.\n");
    }

        // Input nama kota
        char nama_kota[strKota];
        printf("Starting point : ");
        getchar();
        fgets(nama_kota, strKota, stdin);
        nama_kota[strcspn(nama_kota, "\n")] = '\0';

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
                                                                                    
    // Close the file
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

            // Deklarasi utk greedy
            int path[max_index];int visited[max_index];int short_path[15];
            for (int i = 0; i < max_index;i++) {visited[i] = 0;}

            greedy(max_index,start_index,start_index,0,path,short_path,visited,latitude,longitude,0.0,kota);
            
            // Waktu selesai mencari solusi
            clock_t end_time = clock();

            printf("\nBest route :\n");
            for (int i = 0;i < max_index;i++) {
                printf("%s -> ",kota[short_path[i]]);
            }
            printf("%s\n",kota[short_path[0]]);

            printf("\nTotal distance: %.4f km\n", minDist);
             
            // Menghitung waktu yang diperlukan
            double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
            printf("Time elapsed : %.3f s",execution_time);
        }
    return 0;
}