#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace FociSim
{

inline constexpr float kPi = 3.14159265358979323846f;
inline constexpr float kSmallNumber = 1.e-4f;
inline constexpr int32_t kMaxHumanPlayers = 4;

struct Vec2
{
	float X = 0.0f;
	float Y = 0.0f;

	constexpr Vec2() = default;
	constexpr Vec2(float InX, float InY) : X(InX), Y(InY) {}

	constexpr float SizeSquared() const { return X * X + Y * Y; }
	float Size() const { return std::sqrt(SizeSquared()); }

	constexpr Vec2 operator+(const Vec2& Other) const { return Vec2(X + Other.X, Y + Other.Y); }
	constexpr Vec2 operator-(const Vec2& Other) const { return Vec2(X - Other.X, Y - Other.Y); }
	constexpr Vec2 operator*(float Scale) const { return Vec2(X * Scale, Y * Scale); }
};

struct Vec3
{
	float X = 0.0f;
	float Y = 0.0f;
	float Z = 0.0f;

	constexpr Vec3() = default;
	constexpr Vec3(float InX, float InY, float InZ) : X(InX), Y(InY), Z(InZ) {}

	constexpr Vec3 operator+(const Vec3& Other) const { return Vec3(X + Other.X, Y + Other.Y, Z + Other.Z); }
	constexpr Vec3 operator-(const Vec3& Other) const { return Vec3(X - Other.X, Y - Other.Y, Z - Other.Z); }
	constexpr Vec3 operator*(float Scale) const { return Vec3(X * Scale, Y * Scale, Z * Scale); }

	Vec3& operator+=(const Vec3& Other)
	{
		X += Other.X;
		Y += Other.Y;
		Z += Other.Z;
		return *this;
	}

	constexpr float SizeSquared() const { return X * X + Y * Y + Z * Z; }
	float Size() const { return std::sqrt(SizeSquared()); }

	constexpr float SizeSquared2D() const { return X * X + Y * Y; }
	float Size2D() const { return std::sqrt(SizeSquared2D()); }

	Vec3 GetSafeNormal(float Tolerance = kSmallNumber) const
	{
		const float Sq = SizeSquared();
		if (Sq <= Tolerance * Tolerance)
		{
			return Vec3(0.0f, 0.0f, 0.0f);
		}
		const float InvLen = 1.0f / std::sqrt(Sq);
		return Vec3(X * InvLen, Y * InvLen, Z * InvLen);
	}

	Vec3 GetSafeNormal2D(float Tolerance = kSmallNumber) const
	{
		const float Sq = SizeSquared2D();
		if (Sq <= Tolerance * Tolerance)
		{
			return Vec3(0.0f, 0.0f, 0.0f);
		}
		const float InvLen = 1.0f / std::sqrt(Sq);
		return Vec3(X * InvLen, Y * InvLen, 0.0f);
	}

	static constexpr float Dot(const Vec3& A, const Vec3& B)
	{
		return A.X * B.X + A.Y * B.Y + A.Z * B.Z;
	}

	static constexpr Vec3 Cross(const Vec3& A, const Vec3& B)
	{
		return Vec3(
			A.Y * B.Z - A.Z * B.Y,
			A.Z * B.X - A.X * B.Z,
			A.X * B.Y - A.Y * B.X
		);
	}

	static float Dist2D(const Vec3& A, const Vec3& B)
	{
		return (A - B).Size2D();
	}

	static float Dist3D(const Vec3& A, const Vec3& B)
	{
		return (A - B).Size();
	}
};

inline float Clamp(float Value, float MinVal, float MaxVal)
{
	return std::max(MinVal, std::min(MaxVal, Value));
}

inline float Lerp(float A, float B, float Alpha)
{
	return A + (B - A) * Alpha;
}

inline Vec3 Lerp(const Vec3& A, const Vec3& B, float Alpha)
{
	return Vec3(Lerp(A.X, B.X, Alpha), Lerp(A.Y, B.Y, Alpha), Lerp(A.Z, B.Z, Alpha));
}

inline float GetMappedRangeValueClamped(float InMin, float InMax, float OutMin, float OutMax, float Value)
{
	const float Range = InMax - InMin;
	if (std::abs(Range) <= kSmallNumber)
	{
		return OutMin;
	}
	const float Alpha = Clamp((Value - InMin) / Range, 0.0f, 1.0f);
	return Lerp(OutMin, OutMax, Alpha);
}

inline Vec3 StickToBroadcastWorldDir(const Vec2& Stick)
{
	return Vec3(Stick.X, -Stick.Y, 0.0f).GetSafeNormal();
}

enum class TeamSide : uint8_t
{
	Neutral = 0,
	Home = 1,
	Away = 2
};

enum class AssistLevel : uint8_t
{
	Assisted = 0,
	Semi = 1,
	Manual = 2
};

} // namespace FociSim
