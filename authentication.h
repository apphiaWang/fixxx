#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <cstdlib> 


void generate_key_pair(const std::string& name_prefix) {
    int             ret = 0;
    RSA             *r = NULL;
    BIGNUM          *bne = NULL;

    int             bits = 2048;
    unsigned long   e = RSA_F4;
 
    // 1. Generate RSA key
    bne = BN_new();
    ret = BN_set_word(bne, e);
    if(ret != 1){
        // handle error
    }

    r = RSA_new();
    ret = RSA_generate_key_ex(r, bits, bne, NULL);
    if(ret != 1){
        // handle error
    }

    // 2. Save public key
    const char *dir_public_path = "public_keys";
    char public_key_path[256];
    snprintf(public_key_path, sizeof(public_key_path), "%s/%s_public.pem", dir_public_path , name_prefix.c_str());
    BIO *bp_public = BIO_new_file(public_key_path, "w+");
    ret = PEM_write_bio_RSAPublicKey(bp_public, r);
    if(ret != 1){
        // handle error
    }
    
    const char *dir_private_path = "filesystem/private_keys";
    char private_key_path[256];
    snprintf(private_key_path, sizeof(private_key_path), "%s/%s_private.pem", dir_private_path, name_prefix.c_str());
    // 3. Save private key
    BIO *bp_private = BIO_new_file(private_key_path, "w+");
    ret = PEM_write_bio_RSAPrivateKey(bp_private, r, NULL, NULL, 0, NULL, NULL);
    if(ret != 1){
        // handle error
    }

    // 4. Free
    BIO_free_all(bp_public);
    BIO_free_all(bp_private);
    RSA_free(r);
    BN_free(bne);
}

bool validate_login(const std::string& name_prefix, const std::string& data) {
    RSA *private_key = nullptr, *public_key = nullptr;
    BIO *bp_public = nullptr, *bp_private = nullptr;
    unsigned char *encrypted = nullptr, *decrypted = nullptr;
    bool result = false;

    // Initialize variables
    int encrypted_length = 0;
    int decrypted_length = 0;

    // 1. Load private key
    const char *dir_private_path = "filesystem/private_keys";
    char private_key_path[256];
    snprintf(private_key_path, sizeof(private_key_path), "%s/%s_private.pem", dir_private_path, name_prefix.c_str());
    bp_private = BIO_new_file(private_key_path, "r");
    if (!bp_private) {
        // handle error
        std::cerr << "User not found!" << std::endl;
        std::exit(EXIT_FAILURE);
      
    }
    private_key = PEM_read_bio_RSAPrivateKey(bp_private, nullptr, nullptr, nullptr);

    // 2. Encrypt the data with the private key
    int data_len = data.length();
    encrypted = (unsigned char*)malloc(RSA_size(private_key));
    encrypted_length = RSA_private_encrypt(data_len, (unsigned char*)data.c_str(), encrypted, private_key, RSA_PKCS1_PADDING);
    if (encrypted_length == -1) {
        // handle error
       
    }

    // 3. Load public key
    const char *dir_public_path = "public_keys";
    char public_key_path[256];
    snprintf(public_key_path, sizeof(public_key_path), "%s/%s_public.pem", dir_public_path , name_prefix.c_str());
    bp_public = BIO_new_file(public_key_path, "r");
    if (!bp_public) {
        // handle error
      
    }
    public_key = PEM_read_bio_RSAPublicKey(bp_public, nullptr, nullptr, nullptr);

    // 4. Decrypt the data with the public key
    decrypted = (unsigned char*)malloc(encrypted_length);
    decrypted_length = RSA_public_decrypt(encrypted_length, encrypted, decrypted, public_key, RSA_PKCS1_PADDING);
    if (decrypted_length == -1) {
        std::cerr << "Invalid public key!" << std::endl;
        std::exit(EXIT_FAILURE);
       
    }

    // 5. Validate
    result = (data == std::string((char*)decrypted, decrypted_length));

    return result;
}
