# Model Card: TinyMamba (Xmamba Hardware Target)

## Overview
- **Model Name:** TinyMamba (Xmamba Bring-Up Architecture)
- **Model Type:** Selective State Space Sequence Model (Mamba-1 architecture)
- **Role / Ownership:** Member C (Model & Reference)
- **Target Hardware:** VexRiscv RISC-V 32-bit CPU + Xmamba Custom-0 CFU (LiteX SoC on Digilent Arty A7-100T)

---

## Architectural Dimensions & Specifications

| Parameter | Symbol | Value | Hardware / CFU Significance |
| :--- | :---: | :---: | :--- |
| **Model Dimension (Width)** | $d$ | **4** | Residual stream and token embedding vector width |
| **Number of Layers** | $L$ | **2** | Stacked identical Mamba blocks |
| **Expansion Factor** | $E$ | **2** | Expand ratio for hidden channels |
| **Inner Dimension (Channels)** | $E \cdot d$ | **8** | Convolved & scanned inner feature dimension |
| **SSM State Dimension** | $N$ | **4** | Recurrent hidden state $h$ depth per channel |
| **Convolution Kernel Width** | $K$ | **4** | Causal 1D depthwise convolution history length |
| **Delta ($\Delta$) Projection Rank** | $R$ | **2** | Dimension of rank-reduced $\Delta$ projection |
| **Vocabulary Size** | $V$ | **8** | Discrete token vocabulary size |
| **Total Parameter Count** | - | **636** | Fits inside fast on-chip BRAM / tightly coupled memory |

---

## Weight Tensors and Shapes

Total 23 tensors exported as `.npy` in `model/weights/`:

| Layer / Component | Tensor Name | Shape | Parameters |
| :--- | :--- | :--- | :---: |
| **Embeddings** | `backbone.embeddings.weight` | `[8, 4]` | 32 |
| **Layer 0** | `backbone.layers.0.norm.weight` | `[4]` | 4 |
| | `backbone.layers.0.mixer.in_proj.weight` | `[16, 4]` | 64 |
| | `backbone.layers.0.mixer.conv1d.weight` | `[8, 1, 4]` | 32 |
| | `backbone.layers.0.mixer.conv1d.bias` | `[8]` | 8 |
| | `backbone.layers.0.mixer.x_proj.weight` | `[10, 8]` | 80 |
| | `backbone.layers.0.mixer.dt_proj.weight` | `[8, 2]` | 16 |
| | `backbone.layers.0.mixer.dt_proj.bias` | `[8]` | 8 |
| | `backbone.layers.0.mixer.A_log` | `[8, 4]` | 32 |
| | `backbone.layers.0.mixer.D` | `[8]` | 8 |
| | `backbone.layers.0.mixer.out_proj.weight` | `[4, 8]` | 32 |
| **Layer 1** | `backbone.layers.1.norm.weight` | `[4]` | 4 |
| | `backbone.layers.1.mixer.in_proj.weight` | `[16, 4]` | 64 |
| | `backbone.layers.1.mixer.conv1d.weight` | `[8, 1, 4]` | 32 |
| | `backbone.layers.1.mixer.conv1d.bias` | `[8]` | 8 |
| | `backbone.layers.1.mixer.x_proj.weight` | `[10, 8]` | 80 |
| | `backbone.layers.1.mixer.dt_proj.weight` | `[8, 2]` | 16 |
| | `backbone.layers.1.mixer.dt_proj.bias` | `[8]` | 8 |
| | `backbone.layers.1.mixer.A_log` | `[8, 4]` | 32 |
| | `backbone.layers.1.mixer.D` | `[8]` | 8 |
| | `backbone.layers.1.mixer.out_proj.weight` | `[4, 8]` | 32 |
| **Final RMSNorm** | `backbone.norm_f.weight` | `[4]` | 4 |
| **LM Head** | `lm_head.weight` | `[8, 4]` | 32 |
| **TOTAL** | **23 tensors** | - | **636 parameters** |

---

## Task & Evaluation Metrics
- **Task:** Autoregressive Next-Token Sequence Prediction on structured recurrent patterns ($V=8$).
- **Training Objective:** Cross-Entropy Loss with AdamW ($\text{lr}=0.015$).
- **Achieved Accuracy:** **96.00% validation accuracy** (loss: 0.0797).
- **Target Accuracy Metric:** Next-token top-1 exact match, bit-exact trace match across all intermediate stages.
