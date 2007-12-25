/*
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef CHANNEL_CONVERTER_H_
#define CHANNEL_CONVERTER_H_

class ChannelConverter {

    public:
        ChannelConverter() {}
        ChannelConverter(float whiteS, float blackK);
        ~ChannelConverter();

        void KSToReflectance(float K, float S, float &R) const;
        void reflectanceToKS(float R, float &K, float &S) const;
        void RGBTosRGB(float C, float &sC) const;
        void sRGBToRGB(float sC, float &C) const;

    private:
        float Sw, Kb;
        float w0, wi; // For whitening
        float b1;     // For blackening
        float Ke;     // To make S and K continuous in 0.5

        inline float PHI(float R) const { return (2.0*R)/((1.0-R)*(1.0-R)); }
        inline float PSI(float R) const { return  1.0/PHI(R); }

        inline float W(float R) const { return wi*R + w0; }
        inline float B(float R) const { return b1*R; }
};

#endif // CHANNEL_CONVERTER_H_
