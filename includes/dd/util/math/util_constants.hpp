 /*
 *  Copyright (C) W. Michael Knudson
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License along with this program; 
 *  if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
#pragma once

namespace dd::util::math {

    constexpr inline float FloatPi = 3.1415927;

    constexpr inline float Float2Pi = 6.2831855;

    constexpr inline float FloatPiDivided2 = 1.5707964;

    constexpr inline float Float3PiDivided2 = 4.712389;

    constexpr inline float Float1Divided2Pi = 0.15915494;

    constexpr inline float FloatDegree180 = 180.0;

    constexpr inline float FloatQuarternionEpsilon = 1.0E-5;

    /* Ulp stands for "Unit of least precision" as in '1 / 2^23' */
    constexpr inline float FloatUlp = 1.1920929E-7;

    template<typename T, T Degrees> requires std::is_floating_point<T>::value
    constexpr inline T TRadians = (Degrees * FloatPi / FloatDegree180);

    template<typename T, T Radians> requires std::is_floating_point<T>::value
    constexpr inline T TDegrees = (Radians * FloatDegree180 / FloatPi);

    static_assert(TDegrees<float, TRadians<float, 90.0f>> == 90.0f);
    static_assert(TDegrees<float, TRadians<float, 180.0f>> == 180.0f);
    static_assert(TDegrees<float, TRadians<float, 270.0f>> == 270.0f);

    template<typename T, T V, size_t N>
    constexpr inline T TPow = V * TPow<T, V, N-1>;

    template<typename T, T V>
    constexpr inline T TPow<T,V,0> = 1;

    template<typename T, size_t V>
    constexpr inline T TFactorial = V * TFactorial<T, V - 1>;

    template<typename T>
    constexpr inline T TFactorial<T,0> = 1;

    template<double V>
    constexpr inline double TSin = V - (TPow<double, V, 3> / TFactorial<double, 3>) + (TPow<double, V, 5> / TFactorial<double, 5>) - (TPow<double, V, 7> / TFactorial<double, 7>) + (TPow<double, V, 9> / TFactorial<double, 9>) - (TPow<double, V, 11> / TFactorial<double, 11>) + (TPow<double, V, 13> / TFactorial<double, 13>) - (TPow<double, V, 15> / TFactorial<double, 15>);

    template<double V>
    constexpr inline double TCos = 1 - (TPow<double, V, 2> / TFactorial<double, 2>) + (TPow<double, V, 4> / TFactorial<double, 4>) - (TPow<double, V, 6> / TFactorial<double, 6>) + (TPow<double, V, 8> / TFactorial<double, 8>) - (TPow<double, V, 10> / TFactorial<double, 10>) + (TPow<double, V, 12> / TFactorial<double, 12>) - (TPow<double, V, 14> / TFactorial<double, 14>);

    constexpr float SinCoefficients[5] = {(TPow<double, 1.0, 3> / TFactorial<double,3>), (TPow<double, 1.0, 5> / TFactorial<double,5>), (TPow<double, 1.0, 7> / TFactorial<double,7>), (TPow<double, 1.0, 9> / TFactorial<double,9>), (TPow<double, 1.0, 11> / TFactorial<double,11>)};
    constexpr float CosCoefficients[5] = {(TPow<double, 1.0, 2> / TFactorial<double,2>), (TPow<double, 1.0, 4> / TFactorial<double,4>), (TPow<double, 1.0, 6> / TFactorial<double,6>), (TPow<double, 1.0, 8> / TFactorial<double,8>), (TPow<double, 1.0, 10> / TFactorial<double,10>)};

    /* Values used to calculate an angle index for the sincos sample table */
    constexpr inline u32 AngleIndexHalfRound = 0x80000000;
    constexpr inline u32 AngleIndexQuarterRound = 0x40000000;
    constexpr inline u32 AngleIndexThreeQuarterRound = 0xC0000000;

    constexpr long double SlowSin(long double V) {
        return V - (std::pow(V, 3) / TFactorial<long double, 3>) + (std::pow(V, 5) / TFactorial<long double, 5>) - (std::pow(V, 7) / TFactorial<long double, 7>) + (std::pow(V, 9) / TFactorial<long double, 9>) - (std::pow(V, 11) / TFactorial<long double, 11>) + (std::pow(V, 13) / TFactorial<long double, 13>) - (std::pow(V, 15) / TFactorial<long double, 15>) + (std::pow(V, 17) / TFactorial<long double, 17>);
    }

    constexpr long double SlowCos(long double V) {
        return  1.0 - (std::pow(V, 2) / TFactorial<long double, 2>) + (std::pow(V, 4) / TFactorial<long double, 4>) - (std::pow(V, 6) / TFactorial<long double, 6>) + (std::pow(V, 8) / TFactorial<long double, 8>) - (std::pow(V, 10) / TFactorial<long double, 10>) + (std::pow(V, 12) / TFactorial<long double, 12>) - (std::pow(V, 14) / TFactorial<long double, 14>) + (std::pow(V, 16) / TFactorial<long double, 16>);
    }

    consteval auto GenerateSinCosTable() {
        std::array<float, 1024>  values{}; 
        values[0] = std::cos(0);
        values[1] = std::sin(0);

        for(int i = 0; i < 255; ++i) {
            const int index = i * 4;
            const double n = ((2.0 * 3.1415927 * (static_cast<double>(i) + 1.00)) / 256.0);
            double cos_value = SlowCos(n);
            double sin_value = SlowSin(n);
            double cos_diff =  cos_value - values[index];
            double sin_diff =  sin_value - values[index + 1];

            if (-FloatUlp < cos_value && cos_value < FloatUlp) {
                cos_value = 0;
            }
            if (-FloatUlp < sin_value && sin_value < FloatUlp) {
                sin_value = 0;
            }

            values[index + 2] = cos_diff;
            values[index + 3] = sin_diff;
            
            values[index + 4] = cos_value;
            values[index + 5] = sin_value;
        }
        const double cos_value = std::cos((360.0f * FloatPi / FloatDegree180));
        const double sin_value = std::sin((360.0f * FloatPi / FloatDegree180));
        const double cos_diff =  cos_value - values[1020];
        const double sin_diff =  sin_value - values[1021];
        values[1022] = cos_diff;
        values[1023] = sin_diff;

        return values;
    }

    /* Size 1024 float table of cos and sin values interpolated from a period of 2Pi with differences calculated between as implemented above */
    /* Following Cos value, Sin Value, Next Cos Diff, Next Sin Diff, Next Cos value, Next Sin value ... N */
    /* Each method has 256 values and 256 differences */
    const auto SinCosSampleTable = GenerateSinCosTable();

    __attribute__((noinline)) float SampleSin(float value_from_angle_index) {
        const unsigned int angle_index = static_cast<unsigned int>(value_from_angle_index);
        const unsigned int   index     = (angle_index >> 0x18) & 0xFF;
        const float variance = static_cast<float>(angle_index & 0xFFFFFF) * 5.9604644775390625e-8;
        return SinCosSampleTable[(index * 4) + 1] + (SinCosSampleTable[(index * 4) + 3] * variance);
    }

    __attribute__((noinline)) float SampleCos(float value_from_angle_index) {
        const unsigned int angle_index = static_cast<unsigned int>(value_from_angle_index);
        const unsigned int   index     = (angle_index >> 0x18) & 0xFF;
        const float variance = static_cast<float>(angle_index & 0xFFFFFF) * 5.9604644775390625e-8;
        return SinCosSampleTable[(index * 4)] + (SinCosSampleTable[(index * 4) + 2] * variance);
    }

    /* Interpolate a Sin value from one period  */
    __attribute__((noinline)) float GetSinPeriodStep(int t, int max) {
        return SampleSin(((static_cast<float>(AngleIndexHalfRound) / FloatPi) * ((static_cast<float>(t) * Float2Pi) / static_cast<float>(max))));
    }

    /* Interpolate a Cos value from one period  */
    __attribute__((noinline)) float GetCosPeriodStep(int t, int max) {
        return SampleCos(((static_cast<float>(AngleIndexHalfRound) / FloatPi) * ((static_cast<float>(t) * Float2Pi) / static_cast<float>(max))));
    }
}