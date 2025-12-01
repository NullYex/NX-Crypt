# 🔒 NX-Crypt: Smart File Encryption Tool

### By NullYex Team

![C++](https://img.shields.io/badge/Language-C++-blue.svg) ![Security](https://img.shields.io/badge/Security-Salted%20Hash-green.svg) ![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20MacOS-lightgrey.svg)

**NX-Crypt** is a robust, command-line interface (CLI) tool designed to securely encrypt and decrypt files of any type (images, videos, documents, etc.) using advanced C++ file handling and cryptographic logic.

---
### Designed by **NullYex Team** from **B.Tech AI & DS (IBM)**.
### 👥 Authors (NullYex Team)

* 👨‍💻 **Amrit Ranjan**
* 👨‍💻 **Kamran Alvi**
* 👨‍💻 **Farhan Khalid**

---

## ✨ Features

* **🧠 Smart Detection:** Automatically detects if a file is already encrypted and switches modes (Encrypt vs Decrypt).
* **🛡️ Salted Hashing Security:** Uses a unique "Salt" + Hash verification system. We verify your password *without* storing it, protecting your data even if the file header is analyzed.
* **🌀 Rolling Key Algorithm:** The encryption key changes for every single byte processed. Even if the file contains empty space (zeros), the output looks like random garbage.
* **🏷️ Custom Branding:** Injects the `<By_NullYex>` signature to ensure file integrity and brand recognition.
* **⚡ Optimized Performance:** Reads/Writes in **1MB Chunks**, making it incredibly fast for large files (1GB+).
* **💾 Auto-Extension Handling:** Automatically hides the original extension (e.g., `.png`) inside the encrypted file and restores it upon decryption.

---

## 🚀 How It Works

1. **Input:** You provide a file path.
2. **Detection:** The tool checks for the `By_NullYex` signature.
3. **Security:**
   * **Encrypt:** Generates a random Salt, Hashes your password, and creates a secure header.
   * **Decrypt:** Reads the Salt, Re-Hashes your input, and verifies it against the stored Hash.
4. **Processing:** Applies a **Vigenère-style XOR Cipher** with a rolling index key to scramble/unscramble the data.

---

## 🛠️ Installation & Usage

### 1. Compile the Code

You need a C++ compiler (like G++).

```bash
g++ main.cpp -o NX-Crypt
```

### 2. Run the Tool

```bash
./NX-Crypt
```

*(On Windows, just double-click `NX-Crypt.exe`)*

### 3. Follow the Prompts

The tool supports "Copy as Path" from Windows for easy file selection.

```text
Enter file path: "C:\Users\Data\secret.mp4"

Detected: Standard file (Not encrypted).
Mode: ENCRYPTION!

Enter password: mySecurePassword123

Target output: C:\Users\Data\secret.NullYex
Encrypting Please wait...

Success! Operation Completed.
```

**© 2025-26 NullYex Team. All Rights Reserved.**
