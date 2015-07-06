#pragma once
struct SpMat;
struct Vec;
void OptimizeProblem (SpMat &A, Vec &x, Vec &y);
void SpMV (const SpMat &A, const Vec &x, const Vec &y);
