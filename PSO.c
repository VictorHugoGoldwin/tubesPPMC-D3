/*EL2208 Praktikum Pemecahan Masalah dengan C 2023/2024
*Topik            : Tugas Besar
*Hari dan Tanggal : Kamis dan 17 Mei 2024
*Nama (NIM)       : Kean Malik Aji Santoso (13222083)
*Nama File        : PSO.c
*Deskripsi        : 
Program menyelesaikan TSP dengan algoritma PSO
*/

// Penggunaan library di C
#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Definisi Macros
#define STR_KOTA 25
#define STR_FILE 12
#define LEN_LINE 40
#define R 6371
#define C1 2
#define C2 2
#define INERTIA_MAX 0.9
#define STAG_INTERVAL 1000

typedef struct City {
    char *name;
    int id;
} City;

typedef struct Particle {
    int *pos;
    int *bestPos;
    double fitnessVal;
    double *velocity;
} Particle;

double degtoRad(double degree)
{
    double radian = (degree *  M_PI)/180.0;
    return radian;
}

int factorial(int num){
    int result = 1;
    for(int i = 2; i <= num; i++){
        result *= i;
    }
    return result;
}

double calcDistance(double lat1, double lat2, double long1, double long2)
{
    return 2*R*asin(sqrt(pow(sin((lat1-lat2)/2),2) + cos(lat1)*cos(lat2)*pow(sin((long1-long2)/2),2)));
}

void readAndAssign(char **nama_file, char ***kotas, double **lat, double **longi, int *fileSize, char **init_kota){
    // Inisiasi variabel untuk membaca file dan menampung data-datanya
    *nama_file = (char*)malloc(STR_FILE*(sizeof(char)));
    char nama_kota[STR_KOTA];
    int valid = 0;
    char temp[40];

    *kotas = (char**)malloc(sizeof(char*));
    *lat = (double*)malloc(sizeof(double));
    *longi = (double*)malloc(sizeof(double));
    *init_kota = (char*)malloc(STR_KOTA*(sizeof(char)));

    printf("Nama file berisi kota untuk dikunjungi: ");
    scanf("%s", *nama_file);
    FILE* file = fopen(*nama_file,"r");

    // Mengambil bagian extension/format file user
    char* token1;
    token1 = strtok(*nama_file,".");
    token1 = strtok(NULL,"\n");

    // Mengecek keberadaan header file input
    if (token1 == NULL){
        printf("Input file tidak memiliki header file.\n");
        exit(0);
    }
    
    // Jika format bukan csv
    if (strcmp(token1,"csv") != 0) {
        printf("Format file bukan csv.\n");
        exit(0);
    }

    // Jika file tidak ada
    if (file == NULL) {
        printf("File yang diberikan tidak ada.\n");
        exit(0);
    }

    // Jika file ada
    // User diminta untuk memasukkan kota yang dijadikan sebagai titik awal untuk TSP
    printf("Starting point: ");
    scanf("%s",nama_kota);

    // Ekstrak data csv
    char *token;
    while(fgets(temp,LEN_LINE,file)) {
        // Alokasi memori untuk kota, latitude, dan longitude       
        if (*fileSize != 0) {
            *kotas = (char**) realloc (*kotas, ((unsigned)(*fileSize) + 1) * sizeof(char*));
            *lat = (double*) realloc (*lat, ((unsigned)(*fileSize) + 1) * sizeof(double));
            *longi = (double*) realloc (*longi, ((unsigned)(*fileSize) + 1) * sizeof(double));
        }
        
        // Baca nama kota
        (*kotas)[(*fileSize)] = (char*)malloc(STR_KOTA * sizeof(char));
        token = strtok(temp,",");
        strcpy((*kotas)[(*fileSize)], token);

        // Mengecek keberadaan nama kota awal yang dimasukkan User di CSV
        if (strcmp(token,nama_kota) == 0){
            valid = 1;
        }

        // Baca latitude
        token = strtok(NULL,",");
        (*lat)[(*fileSize)] = degtoRad(atof(token));

        // Baca longitude
        token = strtok(NULL,"\n");
        (*longi)[(*fileSize)] = degtoRad(atof(token));

        // Iterasi ID (dari 0) yang digunakan untuk assign data ke array
        (*fileSize)++;
    }

    // Data kosong
    if (*fileSize == 0) {
        printf("File yang diberikan kosong.\n");
        exit(0);
    }

    // Hanya ada satu kota
    else if(*fileSize == 1) {
        printf("Kota hanya satu, tidak bisa dioperasikan.\n");
        exit(0);
    }
    
    // Jika kota diinput tidak ada
    if (!valid) {
        printf("Kota awal yang diinput tidak ada dalam file.\n");
        exit(0);
    }

    // Copy nama kota awal yang diinput user ke sebuah string
    strcpy(*init_kota, nama_kota);

    // Menutup file
    fclose(file);
}

void assignAgain(char **kotas, double *lat, double *longi, int size, City **cityNames, double ***distCities){
    // Array yang menyimpan nama-nama kota
    *cityNames = (City*)malloc((unsigned)(size)*(sizeof(City)));

    // Array yang menyimpan jarak antarkota
    *distCities = (double**)malloc((unsigned)(size)*(sizeof(double*)));

    for(int i = 0; i < size; i++){
        // Alokasi memori untuk elemen array-array dari yang telah didefinisikan sebelumnya
        (*distCities)[i] = (double*)malloc((unsigned)(size)*(sizeof(double)));
        (*cityNames)[i].name = (char*)malloc((STR_KOTA)*sizeof(char));

        // Assign kota yang sedang ditinjau sekarang ke array nama kota
        (*cityNames)[i].id = i;
        strcpy((*cityNames)[i].name, kotas[i]);

        // Iterasi untuk mengetahui jarak antara kota yang sedang dicek dengan kota neighbornya
        for(int j = 0; j < size; j++){
            (*distCities)[i][j] = calcDistance(lat[i], lat[j], longi[i], longi[j]);
        }
    }
}

void findIDKota(char *kota, int *id, City *arrCity, int size){
    for(int i = 0; i < size; i++){
        if(strcmp(kota, arrCity[i].name) == 0){
            *id = arrCity[i].id;
            return;
        }
    }
}

void citySwapping(Particle *p, int posNow, int A, int B){
    int temp = p[posNow].pos[A];
    p[posNow].pos[A] = p[posNow].pos[B];
    p[posNow].pos[B] = temp;
}

void updatePos(Particle *p, int posNow, int size, int idInit){
    // Menghasilkan ID secara random
    int cityID1;
    cityID1 = rand () % size;
    int cityID2;
    cityID2 = rand () % size;
    while(p[posNow].pos[cityID1] == idInit){
        cityID1 = rand() % size;
    }
    while(p[posNow].pos[cityID2] == idInit || cityID1 == cityID2){
        cityID2 = rand() % size;
    }
    
    // City swapping
    citySwapping(p, posNow, cityID1, cityID2);
}

void initParticles(Particle **p, int size, int idAwal){
    // Inisiasi data untuk array of partikel
    (*p) = (Particle*)malloc((unsigned)(size)*sizeof(Particle));

    // Iterasi untuk menghasilkan partikel-partikel baru
    for(int i = 0; i < (size); i++){
        // Inisiasi elemen partikel
        (*p)[i].pos = (int*)malloc((unsigned)(size)*sizeof(int));
        (*p)[i].bestPos = (int*)malloc((unsigned)(size)*sizeof(int));
        (*p)[i].velocity = (double*)malloc((unsigned)(size)*sizeof(double));

        // Iterasi awal untuk assign posisi partikel ke kota awal
        for(int j = 0; j < size; j++){
            (*p)[i].pos[j] = j;
        }

        // Swap ID kota awal ke posisi pertama
        int tempIDSwap = (*p)[i].pos[0];
        (*p)[i].pos[0] = (*p)[i].pos[idAwal];
        (*p)[i].pos[idAwal] = tempIDSwap;

        // Iterasi shuffle posisi kota yang dapat dikunjungi oleh setiap partikel
        // Memastikan tidak ada ID kota yang sama dengan ID kota awal yang diinput User
        for(int j = 1; j <  size; j++){
            // Assignment dilakukan dengan menukar posisi kota-kota
            updatePos(*p, i, size, idAwal);
        }

        // Assign array best position sama dengan array position
        for(int j = 0; j < (size); j++){
            (*p)[i].bestPos[j] = (*p)[i].pos[j];
        }

        // Assign velocity awal untuk setiap partikel sebagai nol
        int maxVelocity = size/10;
        for(int j = 0; j < (size); j++){
            (*p)[i].velocity[j] = ((double)rand() / RAND_MAX) * maxVelocity - maxVelocity / 2.0;
        }

        // Assign fitness setiap partikel nilai awalnya
        (*p)[i].fitnessVal = __INT32_MAX__;
    }
}

void evalFitness(Particle *p, double **distCities, int size){
    // Inisiasi data yang digunakan untuk mengevaluasi fitness
    double totalDist = 0;

    // Iterasi untuk menghitung jarak tempuh sampai semuanya telah dikunjungi dan kembali ke kota awal
    for(int i = 0; i < (size); i++){
        int idY = (i + 1) % size;
        totalDist += distCities[p->pos[i]][p->pos[idY]];
    }

    // Assign fitness yang ter-updated ke partikel
    p->fitnessVal = totalDist;
}

void tspPSO(City *arrC, double **arrD, char* kota_awal, int size){
    // Inisiasi data-data untuk PSO
    Particle *arrParticle = NULL;
    Particle gBest;

    // Mendefinisikan iterasi maksimal dan array untuk menyimpan fitness
    // Maksimal iterasi bisa diubah-ubah
    int maxIterations;
    if(factorial(size) > INT_MAX){
        maxIterations = INT_MAX;
    } else {
        maxIterations = 100000000;
    }

    // Inisiasi variabel yang dapat memberhentikan iterasi PSO
    // Kondisi berhenti iterasi PSO: Global Best Fitness bernilai sama untuk iterasi >= size
    size_t arraySize = (size_t)maxIterations;
    double *arrFitness = (double*)malloc(arraySize * sizeof(double));
    int countStagnant = 0;

    // Mencari ID dari kota awal yang telah dipilih
    int idKotaAwal;
    findIDKota(kota_awal, &idKotaAwal, arrC, size);

    // Inisiasi partikel-partikel
    initParticles(&arrParticle, size, idKotaAwal);

    // Iterasi sampai MAX_ITER untuk algoritma PSO
    for(int iterations = 0; iterations < maxIterations; iterations++){
        // Inisiasi awal global best dan beberapa variabel yang dapat menghentikan iterasi PSO
        if(iterations == 0 || gBest.fitnessVal > arrParticle[0].fitnessVal){
            gBest = arrParticle[0];
        }

        // Meng-update fitness, best position, global best, position, dan velocity
        for(int i = 0; i < (size); i++){
            // Menyimpan fitness sekarang (sebelum di-update) untuk menentukan best position
            Particle curr = arrParticle[i];

            // Meng-update fitness setiap partikel
            evalFitness(&(arrParticle[i]), arrD, size);

            // Menentukan personal best position setiap partikel
            if(curr.fitnessVal < arrParticle[i].fitnessVal){
                // Assign personal best fitness dari current partikel yang belum dievaluasi 
                // ke current partikel yang sudah dievaluasi
                arrParticle[i].fitnessVal = curr.fitnessVal;

                // Assign personal best position dari current partikel yang belum dievaluasi 
                // ke current partikel yang sudah dievaluasi
                for(int j = 0; j < (size); j++){
                    arrParticle[i].bestPos[j] = curr.pos[j];
                }
            }

            // Menentukan global best position untuk populasi partikel dan cek stagnansi
            if(arrParticle[i].fitnessVal < gBest.fitnessVal){
                // Assign current partikel sebagai global best particle
                gBest = arrParticle[i];
            }

            // Meng-update velocity dan position
            for(int j = 0; j < (size); j++){
                // Meng-update velocity
                arrParticle[i].velocity[j] = (INERTIA_MAX*arrParticle[i].velocity[j])
                +(C1*((double) rand()/RAND_MAX)*(arrParticle[i].bestPos[j] - arrParticle[i].pos[j]))
                +(C2*((double) rand()/RAND_MAX)*(gBest.pos[j]- arrParticle[i].pos[j]));
                
                // Meng-update posisi dengan menukar urutan kunjungan kota jika velocity tidak bernilai nol
                // Velocity yang tidak bernilai nol berarti kota yang harus dikunjungi belum tepat
                if((int)arrParticle[i].velocity[j] != 0){
                    updatePos(arrParticle, i, size, idKotaAwal);
                }
            }
        }

        // Assign global best fitness ke arrFitness
        arrFitness[iterations] = gBest.fitnessVal;

        // Mengecek stagnansi setiap 1000 iterasi
        if (iterations % STAG_INTERVAL == 0 && iterations > 0){
            // Mengambil nilai fitness yang terkecil pada 1000 iterasi ke n
            double optimumFitness = __INT32_MAX__;
            for(int i = iterations - STAG_INTERVAL; i < iterations; i++){
                if (arrFitness[i] < optimumFitness){
                    optimumFitness = arrFitness[i];
                }
            }

            // Jika nilai fitness global best particle lebih kecil atau sama dengan optimumFitness pada 1000 iterasi ke n,
            // counter stagnansi bertambah
            if(gBest.fitnessVal <= optimumFitness){
                countStagnant++;
            } else {
                countStagnant = 0;
            }
        }

        // Menghentikan loop jika sudah stagnan
        if(countStagnant >= size){
            break;
        }
    }

    // Output: Global Best Solution setelah iterasi sebanyak MAX_ITER
    // Output: Kota yang harus ditempuh
    printf("Kota tempuh:\n");
    for(int i = 0; i < size; i++){
        printf("%d ", gBest.pos[i]);
        printf("%s->", arrC[gBest.pos[i]].name);
    }
    printf("%s", arrC[gBest.pos[0]].name);
    // Output: Jarak terpendek
    printf("\n");
    printf("Jarak terpendek: \n");
    printf("%f\n", gBest.fitnessVal);
    
    // Free arrFitness
    free(arrFitness);
}

void timeRun(clock_t start, clock_t stop){
    double totalTime = (double)(stop-start)/CLOCKS_PER_SEC;
    printf("Time Elapsed: %f s", totalTime);
}

void deallocateMemChar(char **A, int size){
    // Dealokasi memori untuk matriks
    for(int i = 0; i < size; i++){
        free(A[i]);
   }
    free(A);
}

void deallocateMemCity(City *A, int size){
    // Dealokasi memori untuk matriks
    for(int i = 0; i < size; i++){
        free(A[i].name);
   }
    free(A);
}

void deallocateMemDoub(double **A, int size){
    // Dealokasi memori untuk matriks
    for(int i = 0; i < size; i++){
        free((A)[i]);
    }
}

int main()
{
    // Seed random number generator
    srand((unsigned)time(NULL));

    // Inisiasi variabel-variabel yang dipakai untuk pembacaan CSV
    char *nama_file;
    double *latitude;
    double *longitude;
    char **nama_kotas;
    int fileSize = 0;
    char *kota_awal;

    // Baca file CSV dan masukkan data latitude dan longitude ke array-array yang berkaitan
    readAndAssign(&nama_file, &nama_kotas, &latitude, &longitude, &fileSize, &kota_awal);
    
    // Inisiasi dan assign data CSV ke array-array yang dipakai untuk algoritma PSO
    City *arrCities = NULL;
    double **arrDistCities = NULL;
    assignAgain(nama_kotas, latitude, longitude, fileSize, &arrCities, &arrDistCities);

    // Test untuk prosedur distanceCities
    for(int i = 0; i < fileSize; i++){
        printf("%d %s\n", arrCities[i].id, arrCities[i].name);
        for(int j = 0; j < fileSize; j++){
            printf("%f ", arrDistCities[i][j]);
        }
        printf("\n");
    }
    // return 0;

    // TSP: PSO, Output, dan Waktu
    // Memulai pencatatan waktu
    clock_t startTime = clock();
    // Eksekusi algoritma
    tspPSO(arrCities, arrDistCities, kota_awal, fileSize);
    // Pencatatan waktu berhenti
    clock_t endTime = clock();
    // Penghitungan waktu dan output waktu total eksekusi algoritma
    timeRun(startTime, endTime);

    // Dealokasi memori untuk semua variabel untuk pembacaan CSV
    free(nama_file);
    free(kota_awal);
    deallocateMemChar(nama_kotas, fileSize);
    free(latitude);
    free(longitude);

    // Dealokasi memori untuk semua variabel yang digunakan untuk algoritma
    deallocateMemDoub(arrDistCities, fileSize);
    deallocateMemCity(arrCities, fileSize);

    return 0;
}