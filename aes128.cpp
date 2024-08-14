#include <iostream>
#include <vector>
#include <ctime>
#include <algorithm>
#include <random>
#include <string>

using namespace std;

// Prototypes
void SubBytes(vector<vector<vector<int>>>& states, const int sbox[]);
void InvSubBytes(vector<vector<vector<int>>>& states, const int sbox[]);
void ShiftRows(vector<vector<vector<int>>>& states);
void InvShiftRows(vector<vector<vector<int>>>& states);
void MixColumns(vector<vector<vector<int>>>& states);
void InvMixColumns(vector<vector<vector<int>>>& states);
void AddRoundKey(const vector<vector<int>>& round_key, vector<vector<vector<int>>>& states);
int* create_sbox();
vector<vector<vector<int>>>* input_plaintext();
vector<vector<int>>* input_key();
void g(int word[], const int round, const int sbox[]);
void generate_round_keys(vector<vector<vector<int>>>& v, const int sbox[]);
void trim(string& s);
string Display(vector<vector<vector<int>>>& states);
void encryption(vector<vector<vector<int>>>& states, vector<vector<vector<int>>>& keys, const int sbox[]);
void decryption(vector<vector<vector<int>>>& states, vector<vector<vector<int>>>& keys, const int sbox[]);

int main(void)
{
    srand(time(0));
    cout << "Initialising... This may take a few seconds...\n\n";
    
    // Creating a S-Box
    int* sbox = create_sbox();

    // Taking input
    auto states = *(input_plaintext());

    vector<vector<vector<int>>> round_keys;
    round_keys.push_back(*(input_key()));
    generate_round_keys(round_keys, sbox);

    // Encryption
    cout << "\nENCRYPTION:\n\n";
    encryption(states, round_keys, sbox);

    // Decryption
    cout << "\nDECRYPTION:\n\n";
    decryption(states, round_keys, sbox);

    return 0;
}

void SubBytes(vector<vector<vector<int>>>& states, const int sbox[])
{
    for (auto& state: states)
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
                state[i][j] = sbox[state[i][j]];
        }
    }
}

void InvSubBytes(vector<vector<vector<int>>>& states, const int sbox[])
{
    for (auto& state: states)
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                for (int k = 0; k < 256; k++)
                {
                    if (sbox[k] == state[i][j])
                    {
                        state[i][j] = k;
                        break;
                    }
                }
            }
        }
    }
}

void ShiftRows(vector<vector<vector<int>>>& states)
{
    for (auto& state: states)
    {
        int a, b; // Temporary variables
        
        // The second row gets a left shift of 1
        a = state[1][0];
        state[1][0] = state[1][1];
        state[1][1] = state[1][2];
        state[1][2] = state[1][3];
        state[1][3] = a;
        
        // The third row gets a left shift of 2
        a = state[2][0];
        b = state[2][1];
        state[2][0] = state[2][2];
        state[2][1] = state[2][3];
        state[2][2] = a;
        state[2][3] = b;
        
        // The last row gets a left shift of 3
        a = state[3][3];
        state[3][3] = state[3][2];
        state[3][2] = state[3][1];
        state[3][1] = state[3][0];
        state[3][0] = a;
    }
}

void InvShiftRows(vector<vector<vector<int>>>& states)
{
    for (auto& state: states)
    {
        int a, b; // Temporary variables
        
        // The second row gets a right shift of 1
        a = state[1][3];
        state[1][3] = state[1][2];
        state[1][2] = state[1][1];
        state[1][1] = state[1][0];
        state[1][0] = a;
        
        // The third row gets a right shift of 2
        a = state[2][3];
        b = state[2][2];
        state[2][3] = state[2][1];
        state[2][2] = state[2][0];
        state[2][1] = a;
        state[2][0] = b;
        
        // The last row gets a right shift of 3
        a = state[3][0];
        state[3][0] = state[3][1];
        state[3][1] = state[3][2];
        state[3][2] = state[3][3];
        state[3][3] = a;
    }
}

void MixColumns(vector<vector<vector<int>>>& states)
{
    const int matrix[4][4] = {{2, 3, 1, 1},
                              {1, 2, 3, 1},
                              {1, 1, 2, 3},
                              {3, 1, 1, 2}};
    const int mul2[256] = {
        0x00,0x02,0x04,0x06,0x08,0x0a,0x0c,0x0e,0x10,0x12,0x14,0x16,0x18,0x1a,0x1c,0x1e,
        0x20,0x22,0x24,0x26,0x28,0x2a,0x2c,0x2e,0x30,0x32,0x34,0x36,0x38,0x3a,0x3c,0x3e,
        0x40,0x42,0x44,0x46,0x48,0x4a,0x4c,0x4e,0x50,0x52,0x54,0x56,0x58,0x5a,0x5c,0x5e,
        0x60,0x62,0x64,0x66,0x68,0x6a,0x6c,0x6e,0x70,0x72,0x74,0x76,0x78,0x7a,0x7c,0x7e,	
        0x80,0x82,0x84,0x86,0x88,0x8a,0x8c,0x8e,0x90,0x92,0x94,0x96,0x98,0x9a,0x9c,0x9e,
        0xa0,0xa2,0xa4,0xa6,0xa8,0xaa,0xac,0xae,0xb0,0xb2,0xb4,0xb6,0xb8,0xba,0xbc,0xbe,
        0xc0,0xc2,0xc4,0xc6,0xc8,0xca,0xcc,0xce,0xd0,0xd2,0xd4,0xd6,0xd8,0xda,0xdc,0xde,
        0xe0,0xe2,0xe4,0xe6,0xe8,0xea,0xec,0xee,0xf0,0xf2,0xf4,0xf6,0xf8,0xfa,0xfc,0xfe,
        0x1b,0x19,0x1f,0x1d,0x13,0x11,0x17,0x15,0x0b,0x09,0x0f,0x0d,0x03,0x01,0x07,0x05,
        0x3b,0x39,0x3f,0x3d,0x33,0x31,0x37,0x35,0x2b,0x29,0x2f,0x2d,0x23,0x21,0x27,0x25,
        0x5b,0x59,0x5f,0x5d,0x53,0x51,0x57,0x55,0x4b,0x49,0x4f,0x4d,0x43,0x41,0x47,0x45,
        0x7b,0x79,0x7f,0x7d,0x73,0x71,0x77,0x75,0x6b,0x69,0x6f,0x6d,0x63,0x61,0x67,0x65,
        0x9b,0x99,0x9f,0x9d,0x93,0x91,0x97,0x95,0x8b,0x89,0x8f,0x8d,0x83,0x81,0x87,0x85,
        0xbb,0xb9,0xbf,0xbd,0xb3,0xb1,0xb7,0xb5,0xab,0xa9,0xaf,0xad,0xa3,0xa1,0xa7,0xa5,
        0xdb,0xd9,0xdf,0xdd,0xd3,0xd1,0xd7,0xd5,0xcb,0xc9,0xcf,0xcd,0xc3,0xc1,0xc7,0xc5,
        0xfb,0xf9,0xff,0xfd,0xf3,0xf1,0xf7,0xf5,0xeb,0xe9,0xef,0xed,0xe3,0xe1,0xe7,0xe5
    };
    const int mul3[256] = {
        0x00,0x03,0x06,0x05,0x0c,0x0f,0x0a,0x09,0x18,0x1b,0x1e,0x1d,0x14,0x17,0x12,0x11,
        0x30,0x33,0x36,0x35,0x3c,0x3f,0x3a,0x39,0x28,0x2b,0x2e,0x2d,0x24,0x27,0x22,0x21,
        0x60,0x63,0x66,0x65,0x6c,0x6f,0x6a,0x69,0x78,0x7b,0x7e,0x7d,0x74,0x77,0x72,0x71,
        0x50,0x53,0x56,0x55,0x5c,0x5f,0x5a,0x59,0x48,0x4b,0x4e,0x4d,0x44,0x47,0x42,0x41,
        0xc0,0xc3,0xc6,0xc5,0xcc,0xcf,0xca,0xc9,0xd8,0xdb,0xde,0xdd,0xd4,0xd7,0xd2,0xd1,
        0xf0,0xf3,0xf6,0xf5,0xfc,0xff,0xfa,0xf9,0xe8,0xeb,0xee,0xed,0xe4,0xe7,0xe2,0xe1,
        0xa0,0xa3,0xa6,0xa5,0xac,0xaf,0xaa,0xa9,0xb8,0xbb,0xbe,0xbd,0xb4,0xb7,0xb2,0xb1,
        0x90,0x93,0x96,0x95,0x9c,0x9f,0x9a,0x99,0x88,0x8b,0x8e,0x8d,0x84,0x87,0x82,0x81,	
        0x9b,0x98,0x9d,0x9e,0x97,0x94,0x91,0x92,0x83,0x80,0x85,0x86,0x8f,0x8c,0x89,0x8a,
        0xab,0xa8,0xad,0xae,0xa7,0xa4,0xa1,0xa2,0xb3,0xb0,0xb5,0xb6,0xbf,0xbc,0xb9,0xba,
        0xfb,0xf8,0xfd,0xfe,0xf7,0xf4,0xf1,0xf2,0xe3,0xe0,0xe5,0xe6,0xef,0xec,0xe9,0xea,	
        0xcb,0xc8,0xcd,0xce,0xc7,0xc4,0xc1,0xc2,0xd3,0xd0,0xd5,0xd6,0xdf,0xdc,0xd9,0xda,	
        0x5b,0x58,0x5d,0x5e,0x57,0x54,0x51,0x52,0x43,0x40,0x45,0x46,0x4f,0x4c,0x49,0x4a,
        0x6b,0x68,0x6d,0x6e,0x67,0x64,0x61,0x62,0x73,0x70,0x75,0x76,0x7f,0x7c,0x79,0x7a,	
        0x3b,0x38,0x3d,0x3e,0x37,0x34,0x31,0x32,0x23,0x20,0x25,0x26,0x2f,0x2c,0x29,0x2a,
        0x0b,0x08,0x0d,0x0e,0x07,0x04,0x01,0x02,0x13,0x10,0x15,0x16,0x1f,0x1c,0x19,0x1a
    };
    for (auto& state: states)
    {
        vector<vector<int>> copy(4, vector<int>(4));
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                copy[i][j] = 0;
                for (int k = 0; k < 4; k++)
                {
                    int result;
                    if (matrix[i][k] == 1)
                        result = state[k][j];
                    else if (matrix[i][k] == 2)
                        result = mul2[state[k][j]];
                    else
                        result = mul3[state[k][j]];
                    copy[i][j] ^= result;
                }
            }
        }
        state = copy;
    }
}

void InvMixColumns(vector<vector<vector<int>>>& states)
{
    const int matrix[4][4] = {{14, 11, 13, 9},
                              {9, 14, 11, 13},
                              {13, 9, 14, 11},
                              {11, 13, 9, 14}};
    const int mul9[256] = {
        0x00,0x09,0x12,0x1b,0x24,0x2d,0x36,0x3f,0x48,0x41,0x5a,0x53,0x6c,0x65,0x7e,0x77,
        0x90,0x99,0x82,0x8b,0xb4,0xbd,0xa6,0xaf,0xd8,0xd1,0xca,0xc3,0xfc,0xf5,0xee,0xe7,
        0x3b,0x32,0x29,0x20,0x1f,0x16,0x0d,0x04,0x73,0x7a,0x61,0x68,0x57,0x5e,0x45,0x4c,
        0xab,0xa2,0xb9,0xb0,0x8f,0x86,0x9d,0x94,0xe3,0xea,0xf1,0xf8,0xc7,0xce,0xd5,0xdc,
        0x76,0x7f,0x64,0x6d,0x52,0x5b,0x40,0x49,0x3e,0x37,0x2c,0x25,0x1a,0x13,0x08,0x01,
        0xe6,0xef,0xf4,0xfd,0xc2,0xcb,0xd0,0xd9,0xae,0xa7,0xbc,0xb5,0x8a,0x83,0x98,0x91,
        0x4d,0x44,0x5f,0x56,0x69,0x60,0x7b,0x72,0x05,0x0c,0x17,0x1e,0x21,0x28,0x33,0x3a,
        0xdd,0xd4,0xcf,0xc6,0xf9,0xf0,0xeb,0xe2,0x95,0x9c,0x87,0x8e,0xb1,0xb8,0xa3,0xaa,	
        0xec,0xe5,0xfe,0xf7,0xc8,0xc1,0xda,0xd3,0xa4,0xad,0xb6,0xbf,0x80,0x89,0x92,0x9b,	
        0x7c,0x75,0x6e,0x67,0x58,0x51,0x4a,0x43,0x34,0x3d,0x26,0x2f,0x10,0x19,0x02,0x0b,
        0xd7,0xde,0xc5,0xcc,0xf3,0xfa,0xe1,0xe8,0x9f,0x96,0x8d,0x84,0xbb,0xb2,0xa9,0xa0,
        0x47,0x4e,0x55,0x5c,0x63,0x6a,0x71,0x78,0x0f,0x06,0x1d,0x14,0x2b,0x22,0x39,0x30,
        0x9a,0x93,0x88,0x81,0xbe,0xb7,0xac,0xa5,0xd2,0xdb,0xc0,0xc9,0xf6,0xff,0xe4,0xed,
        0x0a,0x03,0x18,0x11,0x2e,0x27,0x3c,0x35,0x42,0x4b,0x50,0x59,0x66,0x6f,0x74,0x7d,	
        0xa1,0xa8,0xb3,0xba,0x85,0x8c,0x97,0x9e,0xe9,0xe0,0xfb,0xf2,0xcd,0xc4,0xdf,0xd6,
        0x31,0x38,0x23,0x2a,0x15,0x1c,0x07,0x0e,0x79,0x70,0x6b,0x62,0x5d,0x54,0x4f,0x46
    };
    const int mul11[256] = {
        0x00,0x0b,0x16,0x1d,0x2c,0x27,0x3a,0x31,0x58,0x53,0x4e,0x45,0x74,0x7f,0x62,0x69,
        0xb0,0xbb,0xa6,0xad,0x9c,0x97,0x8a,0x81,0xe8,0xe3,0xfe,0xf5,0xc4,0xcf,0xd2,0xd9,
        0x7b,0x70,0x6d,0x66,0x57,0x5c,0x41,0x4a,0x23,0x28,0x35,0x3e,0x0f,0x04,0x19,0x12,
        0xcb,0xc0,0xdd,0xd6,0xe7,0xec,0xf1,0xfa,0x93,0x98,0x85,0x8e,0xbf,0xb4,0xa9,0xa2,
        0xf6,0xfd,0xe0,0xeb,0xda,0xd1,0xcc,0xc7,0xae,0xa5,0xb8,0xb3,0x82,0x89,0x94,0x9f,
        0x46,0x4d,0x50,0x5b,0x6a,0x61,0x7c,0x77,0x1e,0x15,0x08,0x03,0x32,0x39,0x24,0x2f,
        0x8d,0x86,0x9b,0x90,0xa1,0xaa,0xb7,0xbc,0xd5,0xde,0xc3,0xc8,0xf9,0xf2,0xef,0xe4,
        0x3d,0x36,0x2b,0x20,0x11,0x1a,0x07,0x0c,0x65,0x6e,0x73,0x78,0x49,0x42,0x5f,0x54,
        0xf7,0xfc,0xe1,0xea,0xdb,0xd0,0xcd,0xc6,0xaf,0xa4,0xb9,0xb2,0x83,0x88,0x95,0x9e,
        0x47,0x4c,0x51,0x5a,0x6b,0x60,0x7d,0x76,0x1f,0x14,0x09,0x02,0x33,0x38,0x25,0x2e,
        0x8c,0x87,0x9a,0x91,0xa0,0xab,0xb6,0xbd,0xd4,0xdf,0xc2,0xc9,0xf8,0xf3,0xee,0xe5,
        0x3c,0x37,0x2a,0x21,0x10,0x1b,0x06,0x0d,0x64,0x6f,0x72,0x79,0x48,0x43,0x5e,0x55,
        0x01,0x0a,0x17,0x1c,0x2d,0x26,0x3b,0x30,0x59,0x52,0x4f,0x44,0x75,0x7e,0x63,0x68,
        0xb1,0xba,0xa7,0xac,0x9d,0x96,0x8b,0x80,0xe9,0xe2,0xff,0xf4,0xc5,0xce,0xd3,0xd8,
        0x7a,0x71,0x6c,0x67,0x56,0x5d,0x40,0x4b,0x22,0x29,0x34,0x3f,0x0e,0x05,0x18,0x13,
        0xca,0xc1,0xdc,0xd7,0xe6,0xed,0xf0,0xfb,0x92,0x99,0x84,0x8f,0xbe,0xb5,0xa8,0xa3
    };
    const int mul13[256] = {
        0x00,0x0d,0x1a,0x17,0x34,0x39,0x2e,0x23,0x68,0x65,0x72,0x7f,0x5c,0x51,0x46,0x4b,
        0xd0,0xdd,0xca,0xc7,0xe4,0xe9,0xfe,0xf3,0xb8,0xb5,0xa2,0xaf,0x8c,0x81,0x96,0x9b,
        0xbb,0xb6,0xa1,0xac,0x8f,0x82,0x95,0x98,0xd3,0xde,0xc9,0xc4,0xe7,0xea,0xfd,0xf0,
        0x6b,0x66,0x71,0x7c,0x5f,0x52,0x45,0x48,0x03,0x0e,0x19,0x14,0x37,0x3a,0x2d,0x20,
        0x6d,0x60,0x77,0x7a,0x59,0x54,0x43,0x4e,0x05,0x08,0x1f,0x12,0x31,0x3c,0x2b,0x26,
        0xbd,0xb0,0xa7,0xaa,0x89,0x84,0x93,0x9e,0xd5,0xd8,0xcf,0xc2,0xe1,0xec,0xfb,0xf6,
        0xd6,0xdb,0xcc,0xc1,0xe2,0xef,0xf8,0xf5,0xbe,0xb3,0xa4,0xa9,0x8a,0x87,0x90,0x9d,
        0x06,0x0b,0x1c,0x11,0x32,0x3f,0x28,0x25,0x6e,0x63,0x74,0x79,0x5a,0x57,0x40,0x4d,
        0xda,0xd7,0xc0,0xcd,0xee,0xe3,0xf4,0xf9,0xb2,0xbf,0xa8,0xa5,0x86,0x8b,0x9c,0x91,
        0x0a,0x07,0x10,0x1d,0x3e,0x33,0x24,0x29,0x62,0x6f,0x78,0x75,0x56,0x5b,0x4c,0x41,
        0x61,0x6c,0x7b,0x76,0x55,0x58,0x4f,0x42,0x09,0x04,0x13,0x1e,0x3d,0x30,0x27,0x2a,
        0xb1,0xbc,0xab,0xa6,0x85,0x88,0x9f,0x92,0xd9,0xd4,0xc3,0xce,0xed,0xe0,0xf7,0xfa,
        0xb7,0xba,0xad,0xa0,0x83,0x8e,0x99,0x94,0xdf,0xd2,0xc5,0xc8,0xeb,0xe6,0xf1,0xfc,
        0x67,0x6a,0x7d,0x70,0x53,0x5e,0x49,0x44,0x0f,0x02,0x15,0x18,0x3b,0x36,0x21,0x2c,
        0x0c,0x01,0x16,0x1b,0x38,0x35,0x22,0x2f,0x64,0x69,0x7e,0x73,0x50,0x5d,0x4a,0x47,
        0xdc,0xd1,0xc6,0xcb,0xe8,0xe5,0xf2,0xff,0xb4,0xb9,0xae,0xa3,0x80,0x8d,0x9a,0x97
    };
    const int mul14[256] = {
        0x00,0x0e,0x1c,0x12,0x38,0x36,0x24,0x2a,0x70,0x7e,0x6c,0x62,0x48,0x46,0x54,0x5a,
        0xe0,0xee,0xfc,0xf2,0xd8,0xd6,0xc4,0xca,0x90,0x9e,0x8c,0x82,0xa8,0xa6,0xb4,0xba,
        0xdb,0xd5,0xc7,0xc9,0xe3,0xed,0xff,0xf1,0xab,0xa5,0xb7,0xb9,0x93,0x9d,0x8f,0x81,
        0x3b,0x35,0x27,0x29,0x03,0x0d,0x1f,0x11,0x4b,0x45,0x57,0x59,0x73,0x7d,0x6f,0x61,
        0xad,0xa3,0xb1,0xbf,0x95,0x9b,0x89,0x87,0xdd,0xd3,0xc1,0xcf,0xe5,0xeb,0xf9,0xf7,
        0x4d,0x43,0x51,0x5f,0x75,0x7b,0x69,0x67,0x3d,0x33,0x21,0x2f,0x05,0x0b,0x19,0x17,
        0x76,0x78,0x6a,0x64,0x4e,0x40,0x52,0x5c,0x06,0x08,0x1a,0x14,0x3e,0x30,0x22,0x2c,
        0x96,0x98,0x8a,0x84,0xae,0xa0,0xb2,0xbc,0xe6,0xe8,0xfa,0xf4,0xde,0xd0,0xc2,0xcc,
        0x41,0x4f,0x5d,0x53,0x79,0x77,0x65,0x6b,0x31,0x3f,0x2d,0x23,0x09,0x07,0x15,0x1b,
        0xa1,0xaf,0xbd,0xb3,0x99,0x97,0x85,0x8b,0xd1,0xdf,0xcd,0xc3,0xe9,0xe7,0xf5,0xfb,
        0x9a,0x94,0x86,0x88,0xa2,0xac,0xbe,0xb0,0xea,0xe4,0xf6,0xf8,0xd2,0xdc,0xce,0xc0,
        0x7a,0x74,0x66,0x68,0x42,0x4c,0x5e,0x50,0x0a,0x04,0x16,0x18,0x32,0x3c,0x2e,0x20,
        0xec,0xe2,0xf0,0xfe,0xd4,0xda,0xc8,0xc6,0x9c,0x92,0x80,0x8e,0xa4,0xaa,0xb8,0xb6,
        0x0c,0x02,0x10,0x1e,0x34,0x3a,0x28,0x26,0x7c,0x72,0x60,0x6e,0x44,0x4a,0x58,0x56,
        0x37,0x39,0x2b,0x25,0x0f,0x01,0x13,0x1d,0x47,0x49,0x5b,0x55,0x7f,0x71,0x63,0x6d,
        0xd7,0xd9,0xcb,0xc5,0xef,0xe1,0xf3,0xfd,0xa7,0xa9,0xbb,0xb5,0x9f,0x91,0x83,0x8d
    };
    for (auto& state: states)
    {
        vector<vector<int>> copy(4, vector<int>(4));
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                copy[i][j] = 0;
                for (int k = 0; k < 4; k++)
                {
                    int result;
                    if (matrix[i][k] == 9)
                        result = mul9[state[k][j]];
                    else if (matrix[i][k] == 11)
                        result = mul11[state[k][j]];
                    else if (matrix[i][k] == 13)
                        result = mul13[state[k][j]];
                    else
                        result = mul14[state[k][j]];
                    copy[i][j] ^= result;
                }
            }
        }
        state = copy;
    }
}

void AddRoundKey(const vector<vector<int>>& round_key, vector<vector<vector<int>>>& states)
{
    // The same function is used for both encryption and decryption
    // This is because XORing the second time (during decryption)
    // nullifies the first XOR (during encryption)

    for (auto& state: states)
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
                state[i][j] ^= round_key[i][j];
        }
    }
}

int* create_sbox()
{
    static int sbox[256];
    for (int i = 0; i < 256; i++)
        sbox[i] = i;
    unsigned seed = 0;
    shuffle(sbox, sbox + 256, default_random_engine(seed));
    return sbox;
}

vector<vector<vector<int>>>* input_plaintext()
{
    string s;
    cout << "Enter plaintext: ";
    getline(cin, s);
    while (s.empty())
    {
        cout << "Plaintext must not be empty! Re-enter: ";
        getline(cin, s);
    }
    const int len = s.length();
    int index = 0;
    static vector<vector<vector<int>>> v;
    int i = 0, j = 0, k = -1;
    while (index < len)
    {
        if (index % 16 == 0)
        {
            k++;
            i = 0;
            j = -1;
            vector<vector<int>> temp(4, vector<int>(4));
            v.push_back(temp);
        }
        j++;
        if (j == 4)
        {
            i++;
            j = 0;
        }
        v[k][j][i] = int(s[index]);
        index++;
    }
    // Fill any remaining space with ' '.
    while (i != 4)
    {
        j++;
        if (j == 4)
        {
            i++;
            if (i == 4)
                break;
            j = 0;
        }
        v[k][j][i] = int(' ');
    }
    return &v;
}

vector<vector<int>>* input_key()
{
    string key;
    cout << "Enter key(16 characters only!): ";
    getline(cin, key);
    while (key.length() != 16)
    {
        cout << "Key must be exactly 16 characters! Re-enter: ";
        getline(cin, key);
    }
    static vector<vector<int>> v(4, vector<int>(4));
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
            v[j][i] = int(key[i * 4 + j]);
    }
    return &v;
} 

void g(int word[], const int round, const int sbox[])
{
    const int round_consts[] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};
    const int rc[] = {round_consts[round], 0, 0, 0};

    // Left shift by 1
    int a = word[0];
    word[0] = word[1];
    word[1] = word[2];
    word[2] = word[3];
    word[3] = a;

    // Substitution
    word[0] = sbox[word[0]];
    word[1] = sbox[word[1]];
    word[2] = sbox[word[2]];
    word[3] = sbox[word[3]];

    // XOR with round constant
    word[0] ^= rc[0];
    word[1] ^= rc[1];
    word[2] ^= rc[2];
    word[3] ^= rc[3];
}

void generate_round_keys(vector<vector<vector<int>>>& v, const int sbox[])
{
    for (int i = 1; i <= 10; i++)
    {
        vector<vector<int>> temp(4, vector<int>(4));
        v.push_back(temp);
        int word[] = {v[i - 1][0][3], v[i - 1][1][3], v[i - 1][2][3], v[i - 1][3][3]};
        g(word, i - 1, sbox);

        v[i][0][0] = v[i - 1][0][0] ^ word[0];
        v[i][1][0] = v[i - 1][1][0] ^ word[1];
        v[i][2][0] = v[i - 1][2][0] ^ word[2];
        v[i][3][0] = v[i - 1][3][0] ^ word[3];

        for (int j = 1; j <= 3; j++)
        {
            v[i][0][j] = v[i - 1][0][j] ^ v[i][0][j - 1];
            v[i][1][j] = v[i - 1][1][j] ^ v[i][1][j - 1];
            v[i][2][j] = v[i - 1][2][j] ^ v[i][2][j - 1];
            v[i][3][j] = v[i - 1][3][j] ^ v[i][3][j - 1];
        }
    }
}

void trim(string& s)
{
    // Removes the excess ' ' at the end
    const int len = s.length();
    for (int i = len - 1; i >= 0; i--)
    {
        if (s[i] != ' ')
            break;
        s.pop_back();
    }
}

string Display(vector<vector<vector<int>>>& states)
{
    string s;
    for (auto& state: states)
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
                s.push_back(char(state[j][i]));
        }
    }
    trim(s);
    return s;
}

void encryption(vector<vector<vector<int>>>& states, vector<vector<vector<int>>>& keys, const int sbox[])
{
    AddRoundKey(keys[0], states);
    string s;
    for (int i = 1; i <= 10; i++)
    {
        SubBytes(states, sbox);
        ShiftRows(states);
        if (i != 10)
            MixColumns(states);
        AddRoundKey(keys[i], states);
        s = Display(states);
        cout << "After round " << i << ": " << s << endl;
    }
    cout << endl << "Final ciphertext: " << s << endl;
}

void decryption(vector<vector<vector<int>>>& states, vector<vector<vector<int>>>& keys, const int sbox[])
{
    AddRoundKey(keys[10], states);
    string s;
    for (int i = 1; i <= 10; i++)
    {
        InvShiftRows(states);
        InvSubBytes(states, sbox);
        AddRoundKey(keys[10 - i], states);
        if (i != 10)
            InvMixColumns(states);
        s = Display(states);
        cout << "After round " << i << ": " << s << endl;
    }
    cout << endl << "Final plaintext: " << s << endl;
}
