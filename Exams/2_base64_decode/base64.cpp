#include "base64.h"
using namespace std;


class base64 {

private:
    unordered_map<uint8_t, uint8_t> alphabet_;
    string input_;
    string decoded_;
    bool needsPadding_;
    uint8_t paddingNumber_;
    uint32_t buffer_;

    void buildAlphabet(){
        unordered_map<uint8_t, uint8_t> m;
        for (uint8_t i = 0; i <= 25; i++)
            m[i] = 65 + i;

        for (uint8_t i = 26; i <= 51; i++)
            m[i] = 71 + i;

        for (uint8_t i = 52; i <= 61; i++)
            m[i] = i - 52 + '0';    // ascii numbers not actual numbers, '0' = 48

        m[62] = '+';
        m[63] = '/';

        for (auto e : m)
            alphabet_[e.second] = e.first;

    }

public:


    base64(string input) : input_(input), buffer_(0) {
        buildAlphabet();
        needsPadding_ = input.length() % 4 != 0 ? true: false;
    };


    void decode() {

        size_t triplets = 0;

        for (size_t i = 0; i < input_.size(); i += 4) {

            uint32_t val = 0;
            // 4 encoded characters
            array<uint8_t, 4> quad_base64 = { input_[i],input_[i + 1], input_[i + 2], input_[i + 3] };

            // 4 decoded characters
            array<uint8_t, 4> quad_decoded_64;
            uint8_t v = 0;
            for (auto& e : quad_base64) {
                if (e == '=')
                    break;
                quad_decoded_64[v] = alphabet_.at(e);   // [] add, .at() throws
                v++;

            }

            // taken from base 64 from base 10, now we have the 24 bits of 8 bits * 3 in val 
            for (int j = 0, k = 3; j < 4; j++, k--) {
                if (quad_base64[j] == '=') {
                    paddingNumber_ = 4 - j;
                    break;

                }
                val += quad_decoded_64[j] * pow(64, k);
            }

            // reading 8 bits per time for 3 times from our 24 bits "buffer"
                uint8_t bits = 24;
                for (uint8_t n = 0; n < 3; n++) {
                    uint8_t x = 0;
                    bits -= 8;
                    x |= (val >> bits);
                    if(x != 0)  // we avoid writing the true zeros of padding 
                        decoded_.push_back(x);
                }


        }

        triplets++;
    }


    string getDecoded() {
        return decoded_;
    }

};



std::string base64_decode(const std::string& input) {
    base64 b(input);
    b.decode();
    return b.getDecoded();
}





int main()
{
    string input = "bGlnaHQgd29yay4=";
    cout << base64_decode(input);
}