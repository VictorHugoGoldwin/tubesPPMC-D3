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
#define LEN_LINE 50
#define R 6371
#define STAG_INTERVAL 1000

typedef struct City {
    char *name;
    int id;
} City;

typedef struct Particle {
    int *pos;
    int *bestPos;
    double fitnessVal;
    int numSwaps;
    int (*velocity)[2];
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
    getchar();
    scanf("%[^\n]s",nama_kota);

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

void swapping(int *arr, int A, int B){
    int temp = arr[A];
    arr[A] = arr[B];
    arr[B] = temp;
}

void copyArrtoArr(int **pBest, int *pCurr, int size){
    for(int i = 0; i < size; i++){
        (*pBest)[i] = pCurr[i];
    }
}

void combineVelocities(int (*combined)[2], int *num_combined, int (*velocity1)[2], int num_swaps1, int (*velocity2)[2], int num_swaps2) {
    // Menambah current particle's velocity swaps
    *num_combined = 0;
    for (int i = 0; i < num_swaps1; i++) {
        combined[*num_combined][0] = velocity1[i][0];
        combined[*num_combined][1] = velocity1[i][1];
        (*num_combined)++;
    }

    // Menambah velocity swaps dari velocity2/velocity baru
    *num_combined = 0;
    for (int i = 0; i < num_swaps2; i++) {
        combined[*num_combined][0] = velocity2[i][0];
        combined[*num_combined][1] = velocity2[i][1];
        (*num_combined)++;
    }
}

void getSwaps(int *target, int *current, int (*swaps)[2], int *num_swaps, int size) {
    // Membuat variabel sementara dan inisiasi dengan element-element current particle's array of city swaps
    int temp[size];
    for(int i = 0; i < size; i++){
        temp[i] = current[i];
    }

    // Menentukan pasangan kota yang akan di-swap
    *num_swaps = 0;
    for (int i = 1; i < size-1; i++) {
        if (temp[i] != target[i]) {
            for (int j = i+1; j < size; j++) {
                if (temp[j] == target[i]) {
                    // Lakukan Swap
                    swapping(temp, j, i);
                    
                    // Catat hasil city swapping
                    // Jika jumlah swaps belum melebihi ukuran array of velocity, cata city swapping
                    if(*num_swaps < (size)){
                        swaps[*num_swaps][0] = i;
                        swaps[*num_swaps][1] = j;
                        (*num_swaps)++;
                    } else {
                        return;
                    }
                    break;
                }
            }
        }
    }
}

void applySwaps(int *pos, int (*swaps)[2], int swapsNum){
    for(int i = 0; i < swapsNum; i++){
        if(swaps[i][0] == 0 || swaps[i][1] == 0) continue;

        swapping(pos, swaps[i][0], swaps[i][1]);
    }
}

void randomPos(int *posArr, int idInit, int size){
    // Menghasilkan ID secara random
    int cityID1, cityID2;
    cityID1 = rand () % size;
    cityID2 = rand () % size;
    while(posArr[cityID1] == idInit){
        cityID1 = rand() % size;
        }
    while(posArr[cityID2] == idInit || posArr[cityID1] == posArr[cityID2]){
        cityID2 = rand() % size;
    }

    // City swapping
    swapping(posArr, cityID1, cityID2);
}

double evalFitness(int *p, double **distCities, int size){
    // Inisiasi data yang digunakan untuk mengevaluasi fitness
    double totalDist = 0;

    // Iterasi untuk menghitung jarak tempuh sampai semuanya telah dikunjungi dan kembali ke kota awal
    for(int i = 0; i < (size); i++){
        int idY = (i + 1) % size;
        totalDist += distCities[p[i]][p[idY]];
    }

    // Assign fitness yang ter-updated ke partikel
    return totalDist;
}

void initParticles(Particle **p, int size, int idAwal){
    // Inisiasi data untuk array of partikel
    (*p) = (Particle*)malloc((unsigned)(size)*sizeof(Particle));

    // Iterasi untuk menghasilkan partikel-partikel baru
    for(int i = 0; i < (size); i++){
        // Inisiasi elemen partikel
        (*p)[i].pos = (int*)malloc((unsigned)(size)*sizeof(int));
        (*p)[i].bestPos = (int*)malloc((unsigned)(size)*sizeof(int));
        (*p)[i].velocity = (int(*)[2])malloc((unsigned)(size)*sizeof(int[2]));

        // Inisiasi position partikel
        // Iterasi awal untuk assign posisi partikel ke kota awal
        for(int j = 0; j < size; j++){
            (*p)[i].pos[j] = j;
        }
        // Mencari posisi ID awal kota pada array of position partikel
        int idAwalPos;
        for(int j = 0; j < size; j++){
            if((*p)[i].pos[j] == idAwal){
                idAwalPos = j;
            }
        }
        // Swap ID kota awal ke posisi pertama
        swapping((*p)[i].pos, idAwalPos, 0);
        // Iterasi shuffle posisi kota yang dapat dikunjungi oleh setiap partikel
        // Memastikan tidak ada ID kota yang sama dengan ID kota awal yang diinput User
        for(int j = 1; j <  size; j++){
            // Assignment dilakukan dengan menukar posisi kota-kota
            // Menghasilkan ID secara random
            randomPos((*p)[i].pos, idAwal, size);
        }

        // Inisiasi personal best position partikel dengan initial position partikel
        copyArrtoArr(&((*p)[i].bestPos), (*p)[i].pos, size);

        // Assign jumlah penukaran order of cities to be visited awal untuk setiap partikel sebagai nol
        (*p)[i].numSwaps = 0;
        
        // Assign fitness setiap partikel nilai awalnya
        (*p)[i].fitnessVal = __INT32_MAX__;
    }
}

void tspPSO(City *arrC, double **arrD, char* kota_awal, int size){
    // Inisiasi data-data untuk PSO
    Particle *arrParticle = NULL;
    Particle gBest;
    int newVelG[size][2];
    int gBestDiff[size][2];

    // Mendefinisikan iterasi maksimal dan array untuk menyimpan fitness
    // Maksimal iterasi bisa diubah-ubah
    int maxIterations;
    if(factorial(size) > INT_MAX){
        maxIterations = INT_MAX;
    } else {
        maxIterations = 1000000;
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
        // Inisiasi global best
        if(iterations == 0){
            gBest = arrParticle[0];
        }

        if(countStagnant <= (size*STAG_INTERVAL)/10){
            // Assign global best fitness ke arrFitness
            arrFitness[iterations] = gBest.fitnessVal;

            // Mengecek stagnansi setiap STAG_INTERVAL*size iterasi
            if (iterations % (STAG_INTERVAL*size) == 0 && iterations > 0){
                // Mengambil nilai fitness yang terkecil pada 1000 iterasi ke n
                double optimumFitness = __INT32_MAX__;
                for(int i = iterations - (size*STAG_INTERVAL); i < iterations; i++){
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

            // Meng-update partikel jika belum stagnan
            for(int i = 0; i < (size); i++){
                // Menyimpan fitness sekarang (sebelum di-update) untuk menentukan best position
                Particle curr = arrParticle[i];

                // Meng-update fitness setiap partikel
                curr.fitnessVal = evalFitness(curr.pos, arrD, size);

                // Menentukan personal best position setiap partikel
                if(curr.fitnessVal < arrParticle[i].fitnessVal){
                    // Assign personal best fitness dari current partikel yang belum dievaluasi 
                    // ke current partikel yang sudah dievaluasi
                    arrParticle[i].fitnessVal = curr.fitnessVal;
                    copyArrtoArr(&(arrParticle[i].bestPos), curr.pos, size);
                }

                // Menentukan global best position untuk populasi partikel
                if(arrParticle[i].fitnessVal < gBest.fitnessVal){
                    // Copy current particle ke partikel global best
                    copyArrtoArr(&(gBest.pos), arrParticle[i].pos, size);
                    copyArrtoArr(&(gBest.bestPos), arrParticle[i].bestPos, size);
                    gBest.fitnessVal = evalFitness(gBest.pos, arrD, size);
                }

                // Randomize global best dan update global best's best position
                // Inisiasi variabel yanng menampung global best sebelum di-random
                Particle currG = gBest;
                // Randomize variabel tersebut
                randomPos(currG.pos, idKotaAwal, size);
                currG.fitnessVal = evalFitness(currG.pos, arrD, size);
                if(currG.fitnessVal < gBest.fitnessVal){
                    // Copy current global best ke partikel global best sebelum di-random
                    gBest.fitnessVal = currG.fitnessVal;
                    copyArrtoArr(&(gBest.bestPos), currG.pos, size);
                }
                
                // Meng-update velocity dan position
                // Inisiasi variabel-variabel untuk velocity update
                int newSwapsG = 0;
                int gBestSwaps = 0;
                for(int j = 0; j < size; j++){
                    newVelG[j][0] = gBestDiff[j][0] = 0;
                    newVelG[j][1] = gBestDiff[j][1] = 0;
                }

                // Lakukan swapping cities berdasarkan global best's position 
                // Mencari pasangan cities yang mungkin di-swap berdasarkan global best position
                getSwaps(gBest.pos, arrParticle[i].pos, gBestDiff, &gBestSwaps, size);

                // Menggabung cities swaps personal best dan cities swaps global best
                combineVelocities(newVelG, &newSwapsG, arrParticle[i].velocity, arrParticle[i].numSwaps, gBestDiff, gBestSwaps);

                // Assign jumlah cities swaps yang harus dilakukan ke current particle
                arrParticle[i].numSwaps = newSwapsG;

                // Assign cities swaps ke current particle's velocity
                for(int j = 0; j < newSwapsG; j++){
                    arrParticle[i].velocity[j][0] = newVelG[j][0];
                    arrParticle[i].velocity[j][1] = newVelG[j][1];
                }

                // Lakukan cities swaps
                applySwaps(arrParticle[i].pos, arrParticle[i].velocity, arrParticle[i].numSwaps);
            }   
        } else {
            // Menghentikan loop jika sudah stagnan
            break;
        }
    }

    // Output: Global Best Solution setelah iterasi sebanyak MAX_ITER
    // Output: Kota yang harus ditempuh
    printf("Kota tempuh:\n");
    for(int i = 0; i < size; i++){
        printf("%s->", arrC[gBest.bestPos[i]].name);
    }
    printf("%s\n", arrC[gBest.bestPos[0]].name);
    // Output: Jarak terpendek
    printf("Jarak terpendek dari kota tempuh: \n");
    printf("%f\n", evalFitness(gBest.bestPos, arrD, size));
    printf("Jarak terpendek yang seharusnya: \n");
    printf("%f\n", gBest.fitnessVal);
    
    // Free arrFitness dan arrParticle
    free(arrParticle);
    free(arrFitness);
}

void timeRun(clock_t start, clock_t stop){
    double totalTime = (double)(stop-start)/CLOCKS_PER_SEC;
    printf("Time Elapsed: %f s", totalTime);
}

void deallocateMemChar(char ***A, int size){
    // Dealokasi memori untuk matriks
    for(int i = 0; i < size; i++){
        free((*A)[i]);
   }
    free(*A);
    *A = NULL;
}

void deallocateMemCity(City **A, int size){
    // Dealokasi memori untuk matriks
    for(int i = 0; i < size; i++){
        free((*A)[i].name);
   }
    free(*A);
    *A = NULL;
}

void deallocateMemDoub(double ***A, int size){
    // Dealokasi memori untuk matriks
    for(int i = 0; i < size; i++){
        free((*A)[i]);
    }
    free(*A);
    *A = NULL;
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
    deallocateMemChar(&nama_kotas, fileSize);
    free(latitude);
    free(longitude);

    // Dealokasi memori untuk semua variabel yang digunakan untuk algoritma
    deallocateMemDoub(&arrDistCities, fileSize);
    deallocateMemCity(&arrCities, fileSize);

    return 0;
}