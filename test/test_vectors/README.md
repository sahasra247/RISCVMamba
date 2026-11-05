# Test Vectors (Member C Task)

Inputs and outputs for 10 tokens across all intermediate layers and stages.
Layout:
- `tokens`: [1, 4, 6, 1, 3, 4, 6, 0, 4, 1]
- `stages`: RMSNorm, In_Proj, Conv+SiLU, X_Proj, Delta+Softplus, Scan, Gate, Out_Proj, Residual, FinalNorm, LMHead
Values match the bit-exact integer contract implemented on the Xmamba CFU core.
