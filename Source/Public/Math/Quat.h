#pragma once

template <typename T>
struct TQuat
{
    union
    {
        struct // NOLINT
        {
            T X;
            T Y;
            T Z;
            T W;
        };
        T XYZW[4];
    };

    TQuat() : X(0), Y(0), Z(0), W(0)
    {
    }
    TQuat(T InX, T InY, T InZ, T InW) : X(InX), Y(InY), Z(InZ), W(InW)
    {
    }
    TQuat(TVector3<T> Axis, T Angle)
    {
        const T HalfAngle = 0.5f * Angle;
        T S, C;
        Math::SinCos(&S, &C, HalfAngle);

        X = S * Axis.X;
        Y = S * Axis.Y;
        Z = S * Axis.Z;
        W = C;
    }

    TQuat(const TMatrix<T>& M)
    {
        //const MeReal *const t = (MeReal *) tm;
        T s;

        // Check diagonal (trace)
        const T tr = M.M[0][0] + M.M[1][1] + M.M[2][2];

        if (tr > 0.0f)
        {
            T InvS = Math::InvSqrt(tr + 1.f);
            this->W = 0.5f * (1.f / InvS);
            s = 0.5f * InvS;

            this->X = ((M.M[1][2] - M.M[2][1]) * s);
            this->Y = ((M.M[2][0] - M.M[0][2]) * s);
            this->Z = ((M.M[0][1] - M.M[1][0]) * s);
        }
        else
        {
            // diagonal is negative
            int32 i = 0;

            if (M.M[1][1] > M.M[0][0])
                i = 1;

            if (M.M[2][2] > M.M[i][i])
                i = 2;

            static constexpr int32 nxt[3] = {1, 2, 0};
            const int32 j = nxt[i];
            const int32 k = nxt[j];

            s = M.M[i][i] - M.M[j][j] - M.M[k][k] + 1.0f;

            T InvS = Math::InvSqrt(s);

            T qt[4];
            qt[i] = 0.5f * (1.f / InvS);

            s = 0.5f * InvS;

            qt[3] = (M.M[j][k] - M.M[k][j]) * s;
            qt[j] = (M.M[i][j] + M.M[j][i]) * s;
            qt[k] = (M.M[i][k] + M.M[k][i]) * s;

            this->X = qt[0];
            this->Y = qt[1];
            this->Z = qt[2];
            this->W = qt[3];
        }
    }
    
    TRotator<T> Rotator();
};

template <>
FRotator FQuat::Rotator();
