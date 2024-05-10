/*EL2208 Praktikum Pemecahan Masalah dengan C 2023/2024
*Modul            : 9 - Tugas Besar
*Hari dan Tanggal : Jumat, 10 Mei 2024
*Nama (NIM)       : Victor Hugo Goldwin (13222085)
*Asisten (NIM)    : Isnaini Azhar Ramadhan Wijaya (18321016)
*Nama File        : DFS.c
*Deskripsi        : Membuat program menyelesaikan Travelling Salesman Problem dengan DFS (Mix Bruteforce)
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

void DFS(int max_index,int start_index,int current,int depth,int path[],int short_path[],int visited[],double latitude[],double longitude[],double distance,char** kota) 
{
    visited[current] = 1;
    path[depth] = current;
    // Base dari rekursif, yaitu ketika sudah mencapai node paling ujung dari traversal
    if (depth == max_index-1) {
        // Menambah jarak kembali dari kota terakhir kembali ke kota awal
        distance += calcDistance(latitude[current],latitude[start_index],longitude[current],longitude[start_index]);
        if (distance < minDist) {
            minDist = distance;
            for (int i = 0;i < max_index;i++) {
                short_path[i] = path[i];
            }
        }
    }
    else {
        double temp_distance = distance;
        for (int nextCity = 0; nextCity < max_index; nextCity++) {
            if (!visited[nextCity] && distance + calcDistance(latitude[current],latitude[nextCity],longitude[current],longitude[nextCity]) < minDist) {
                distance += calcDistance(latitude[current],latitude[nextCity],longitude[current],longitude[nextCity]);
                DFS(max_index,start_index,nextCity,depth + 1,path,short_path,visited,latitude,longitude,distance,kota);
                distance = temp_distance;
            }
        }
    }
    visited[current] = 0;
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

            // Deklarasi Variabel untuk DFS
            int path[max_index];int visited[max_index];int short_path[4];
            for (int i = 0; i < max_index;i++) {visited[i] = 0;}

            // Rekursif DFS-Bruteforce
            DFS(max_index,start_index,start_index,0,path,short_path,visited,latitude,longitude,0.0,kota);
            
            // Waktu selesai mencari solusi
            clock_t end_time = clock();

            // Output path
            printf("\nBest route :\n");
            for (int i = 0;i < max_index;i++) {
                printf("%s -> ",kota[short_path[i]]);
            }
            printf("%s\n",kota[start_index]);
            printf("\nTotal distance : %.4f km\n",minDist);

            // Menghitung waktu yang diperlukan
            double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
            printf("Time elapsed : %.3f s",execution_time);
        }
    }
}