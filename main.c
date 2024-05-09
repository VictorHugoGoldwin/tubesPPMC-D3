#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define strKota 25
#define r 6371

double degtoRad(double degree)
{
    return degree * M_PI / 180.0;
}

double calcDistance(double lat1, double lat2, double long1, double long2)
{
    return 2*r*asin(sqrt(pow(sin((lat1-lat2)/2),2) + cos(lat1)*cos(lat2)*pow(sin((long1-long2)/2),2)));
}

double tsp(double latitude[],double longitude[]) {}

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
    
    // Jika format bukan csv
    if (strcmp(token,"csv") != 0) {printf("\nFormat file bukan csv.\n");}
    // Jika file tidak ada
    else if (file == NULL) {printf("\nFile yang diberikan tidak ada.\n");}
    else {
        // Input nama kota
        char nama_kota[strKota];
        printf("Starting point : ");
        scanf("%s",nama_kota);

        char temp[40];
        // Deklarasi variabel untuk operasi ekstrak data csv
        int valid = 0;int i = 0;
        // Ekstrak data csv
        while(fgets(temp,40,file)) {
            // Alokasi memori untuk kota,latitude,longitude
            if (i == 0) {
                kota = (char**) malloc (sizeof(char*));
                kota[i] = (char*) malloc (strKota * sizeof(char));
                latitude = (double*) malloc (sizeof(double));
                longitude = (double*) malloc (sizeof(double));
            }        
            else {
                kota = (char**) realloc (kota, (i+1)*sizeof(char*));
                kota[i] = (char*) malloc (strKota * sizeof(char));
                latitude = (double*) realloc (latitude, (i+1)*sizeof(double));
                longitude = (double*) realloc (longitude, (i+1)*sizeof(double));
            }

            token = strtok(temp,",");
            strcpy(kota[i],token);
            // Mengecek nama kota ada di list atau tidak
            if (strcmp(token,nama_kota) == 0) {valid = 1;}

            token = strtok(NULL,",");
            latitude[i] = degtoRad(atof(token));
            token = strtok(NULL,"\n");
            longitude[i] = degtoRad(atof(token));
            i++;
        }

        // Data kosong
        if (i == 0) {printf("\nFile yang diberikan kosong.\n");}
        
        // Jika kota diinput tidak ada
        if (!valid) {printf("\nKota yang diinput tidak ada dalam file.\n");}
        // Jika kota diinput ada (Nanti ini diganti program TSP sesuai algoritma masing-masing)
        // Isi else ini dengan program TSP kalian untuk nyari jarak terdekat nya ya, bebas mau gimana, ini contoh hitung doang
        else {
            // Waktu mulai mencari solusi
            clock_t start_time = clock();
            double sum = 0;
            printf("\nBest route : ");
            // Ganti proses TSP Algoritma Masing-masing ya bawah ini
            for (int j = 0;j < i+1;j++) {
                // Alokasi memori untuk menampung jarak terdekat antar kota
                distance = (double*) malloc ((i+1)*sizeof(double));
                // Jarak dalam km
                distance[0] = calcDistance(latitude[0],latitude[2],longitude[0],longitude[2]);
                distance[1] = calcDistance(latitude[2],latitude[3],longitude[2],longitude[3]);
                distance[2] = calcDistance(latitude[1],latitude[3],longitude[1],longitude[3]);
                distance[3] = calcDistance(latitude[1],latitude[0],longitude[1],longitude[0]);
                sum += distance[j];
            }
            // Waktu selesai mencari solusi
            clock_t end_time = clock();

            // Print route dan jarak
            printf("\n%s -> %s -> %s -> %s -> %s\n",kota[0],kota[2],kota[3],kota[1],kota[0]);
            printf("Jarak terdekat : %f\n",sum);

            // Menghitung waktu yang diperlukan
            double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
            printf("Time elapsed : ");
        }
    }
}