#include "tx.h"

Tx::Tx(int version, vector<TxIn> tx_ins, vector<TxOut> tx_outs,
    unsigned int locktime, bool is_testnet) {
    this->version = version;
    this->tx_ins = tx_ins;
    this->tx_outs = tx_outs;
    this->locktime = locktime;
    this->is_testnet = is_testnet;
}

Tx::Tx() {}

bool Tx::parse(vector<uint8_t>& d) {
    // https://en.bitcoin.it/wiki/Protocol_documentation#tx
    if (d.size() < 60) {
        cerr << "Tx::parse() failed: byte vector doesn't contain "
             << "expected number of bytes. \n"
             << "Expect: >= 60\n"
             << "Actual: " << d.size() << "\n"
             << "the Tx instance is corrupted\n";
        return false;
    }
    version = (d[0] << 0 | d[1] << 8 | d[2] << 16 | d[3] << 24);
    d.erase(d.begin(), d.begin() + 4);

    if (d[0] == 0 && d[1] == 1) {
        // BIP141
        witness_flag = true;
        d.erase(d.begin(), d.begin() + 2);
        cout << "witness flag set" << endl;
    }

    tx_in_count = read_variable_int(d);
    for (size_t i = 0; i < tx_in_count; ++i) {
        TxIn tx_in = TxIn();
        if (!tx_in.parse(d)) {
            fprintf(stderr, "Tx::parse() failed: %lu-th tx_in.parse() failed. "
            "The Tx instance is corrupted\n", i);
            return false;
        }
        tx_ins.push_back(tx_in);
    }
    tx_out_count = read_variable_int(d);
    for (size_t i = 0; i < tx_out_count; ++i) {
        TxOut tx_out = TxOut();
        tx_out.parse(d);
        tx_outs.push_back(tx_out);
    }

    if (witness_flag) {
        witeness_count = read_variable_int(d);
        witenesses = vector<vector<uint8_t>>(witeness_count);
        for (size_t i = 0; i < witeness_count; ++i) {
            size_t witeness_size = read_variable_int(d);
            witenesses[i] = vector<uint8_t>(witeness_size);
            memcpy(witenesses[i].data(), d.data(), witeness_size);
            d.erase(d.begin(), d.begin() + witeness_size);
        }
    }

    if (d.size() == 4) {        
        locktime = (d[0] << 0 | d[1] << 8 | d[2] << 16 | d[3] << 24);
        d.erase(d.begin(), d.begin() + 4);
    } else {
        fprintf(stderr, "Tx::parse() failed: byte vector doesn't contain "
            "expected number of bytes.\nExpect: 4\nActual: %lu\nThe Tx "
            "instance is corrupted\n", d.size());
        return false;
    }
    return true;
}


int Tx::fetch_tx(const uint8_t tx_id[SHA256_HASH_SIZE], vector<uint8_t>& d) {
    unique_char_ptr tx_id_hex(bytes_to_hex_string(
        tx_id, SHA256_HASH_SIZE, false));
    json data = bitcoind_rpc(
        R"({
            "jsonrpc": "1.0",
            "method": "getrawtransaction",
            "params": [")" + string(tx_id_hex.get()) + R"("]
        })");
    if (data["result"].is_null()) {
        fprintf(stderr, "bitcoind_rpc() failed: %s\n", data.dump().c_str());
        return 3;
    }
    string tx_hex = data["result"].get<string>();
    d.resize(tx_hex.size() + 1);
    memcpy(d.data(), tx_hex.data(), tx_hex.size());
    d[tx_hex.size()] = '\0';
    return 0;
}

uint32_t Tx::get_version() {
    return version;
}

uint32_t Tx::get_tx_in_count() {
    return tx_in_count;
}

uint32_t Tx::get_tx_out_count() {
    return tx_out_count;
}

vector<TxIn> Tx::get_tx_ins() {
    return tx_ins;
}

vector<TxOut> Tx::get_tx_outs() {
    return tx_outs;
}

uint32_t Tx::get_locktime() {
    return locktime;
}

uint32_t Tx::get_fee() {
    uint32_t input_sum = 0, output_sum = 0;
    for (size_t i = 0; i < tx_in_count; ++i) {
        input_sum += tx_ins[i].get_value();
    }
    for (size_t i = 0; i < tx_out_count; ++i) {
        output_sum += tx_outs[i].get_amount();
    }
    return input_sum - output_sum;
}

Tx::~Tx() {}



TxIn::TxIn(const uint8_t* prev_tx_id, uint32_t prev_tx_idx,
    void* script_sig, uint32_t sequence) {
    memcpy(this->prev_tx_id, prev_tx_id, SHA256_HASH_SIZE);
    this->prev_tx_idx = prev_tx_idx;
    this->script_sig = script_sig;
    this->sequence = sequence;
}

TxIn::TxIn() {}

bool TxIn::parse(vector<uint8_t>& d) {
    if (d.size() < SHA256_HASH_SIZE) {
        fprintf(stderr, "TxIn::parse() failed: byte vector doesn't contain "
            "expected number of bytes. \nExpect: %d\nActual: %lu\n. This TxIn "
            "instance is corrupted\n", SHA256_HASH_SIZE, d.size());
        return false;
    }
    memcpy(prev_tx_id, d.data(), SHA256_HASH_SIZE);
    d.erase(d.begin(), d.begin() + SHA256_HASH_SIZE);
    
    reverse(prev_tx_id, prev_tx_id + SHA256_HASH_SIZE);
    if (d.size() < 4) {
        fprintf(stderr, "TxIn::parse() failed: byte vector doesn't contain "
            "expected number of bytes. \nExpect: 4\nActual: %lu\n. This TxIn "
            "instance is corrupted\n", d.size());
        return false;
    }
    prev_tx_idx = (d[0] << 0 | d[1] << 8 | d[2] << 16 | d[3] << 24);
    d.erase(d.begin(), d.begin() + 4);
    uint64_t script_len = read_variable_int(d);
    d.erase(d.begin(), d.begin() + script_len);
    // The parsing of script will be skipped for the time being.

    if (d.size() < 4) {
        fprintf(stderr, "TxIn::parse() failed: byte vector doesn't contain "
            "expected number of bytes. \nExpect: 4\nActual: %lu\n. This TxIn "
            "instance is corrupted\n", d.size());
        return false;
    }
    sequence = (d[0] << 0 | d[1] << 8 | d[2] << 16 | d[3] << 24);
    d.erase(d.begin(), d.begin() + 4);
    // The sequence field doesn't appears to be useful for Bitcoin's operation now due to security concerns.
    return true;
}

uint8_t* TxIn::get_prev_tx_id() {
    return prev_tx_id;
}

uint32_t TxIn::get_prev_tx_idx() {
    return prev_tx_idx;
}

uint32_t TxIn::get_sequence() {
    return sequence;
}

uint64_t TxIn::get_value() {
    Tx tx = Tx();
    vector<uint8_t> d(64);
    if (Tx::fetch_tx(get_prev_tx_id(), d) != 0) {
        fprintf(stderr, "Failed to Tx::fetch() tx_id\n");
        return 0;
    }
    int64_t hex_len;
    unique_byte_ptr hex_input(hex_string_to_bytes((char*)d.data(), &hex_len));
    d.resize(hex_len); // let's re-use the same vector...
    memcpy(d.data(), hex_input.get(), hex_len);
    tx.parse(d);
    vector<TxOut> tx_outs = tx.get_tx_outs();
    return tx_outs[get_prev_tx_idx()].get_amount();
}

TxIn::~TxIn() {
}


TxOut::TxOut(const uint64_t amount, void* script_pubkey) {
    this->amount = amount;
    this->script_pubkey = script_pubkey;
}

TxOut::TxOut() {}

void TxOut::parse(vector<uint8_t>& d) {
    uint8_t buf[8];
    memcpy(buf, d.data(), 8);
    d.erase(d.begin(), d.begin() + 8);
    amount = (
        buf[0] << 0 | buf[1] << 8 | buf[2] << 16 | buf[3] << 24
    );
    uint64_t script_len = read_variable_int(d);

    d.erase(d.begin(), d.begin() + script_len);
    // The parsing of script will be skipped for the time being.
}

uint64_t TxOut::get_amount() {
    return amount;
}

uint8_t* serialize() {
    return (uint8_t*)nullptr;
}

TxOut::~TxOut() {
}
