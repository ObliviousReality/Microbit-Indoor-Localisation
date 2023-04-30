// #include "MicroBit.h"
// #include "fft.h"

// FFT::FFT() {}

// void FFT::process()
// {
//     unsigned int N = data.size(), k = N, n;
//     double thetaT = PI / N;
//     Complex phit = Complex(cos(thetaT), -sin(thetaT)), T;
//     while (k > 1)
//     {
//         n = k;
//         k >>= 1;
//         phit *= phit;
//         T = 1.0L;
//         for (int l = 0; l < k; l++)
//         {
//             for (int a = 1; a < N; a += n)
//             {
//                 int b = a + k;
//                 Complex t = data[a] - data[b];
//                 data[a] += data[b];
//                 data[b] = t * T;
//             }
//             T *= phit;
//         }
//     }
//     int m = (int)log2(N);
//     for (int a = 0; a < N; a++)
//     {
//         int b = a;
//         b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
//         b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
//         b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
//         b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
//         b = ((b >> 16) | (b << 16)) >> (32 - m);
//         if (b > a)
//         {
//             Complex t = data[a];
//             data[a] = data[b];
//             data[b] = t;
//         }
//     }
// }
