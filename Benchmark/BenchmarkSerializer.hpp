#include "Serializer.hpp"
#include <benchmark/benchmark.h>
#include "json/json.h"
#include <random>

constexpr int vectorSize = 10000;

struct Profile
{
    int32_t id{};
    double score{};
    std::string name;
    std::vector<int32_t> tags;
    std::map<std::string, int64_t> kv;

    auto Tie()
    {
        return std::tie(id, score, name, tags, kv);
    }
    auto Tie() const
    {
        return std::tie(id, score, name, tags, kv);
    }
};

static Json::Value ToJson(const Profile &p)
{
    Json::Value j;
    j["id"] = p.id;
    j["score"] = p.score;
    j["name"] = p.name;
    for (auto t : p.tags)
        j["tags"].append(t);
    for (auto &kv : p.kv)
        j["kv"][kv.first] = Json::Int64(kv.second);
    return j;
}

static Profile FromJson(const Json::Value &j)
{
    Profile p;
    p.id = j["id"].asInt();
    p.score = j["score"].asDouble();
    p.name = j["name"].asString();
    for (auto &t : j["tags"])
        p.tags.push_back(t.asInt());
    auto kvs = j["kv"].getMemberNames();
    for (auto &k : kvs)
        p.kv[k] = j["kv"][k].asInt64();
    return p;
}

static Profile MakeRandomProfile(std::mt19937_64 &rng)
{
    std::uniform_int_distribution<int32_t> di(1, 1000000);
    std::uniform_real_distribution<double> dd(0.0, 10000.0);
    std::uniform_int_distribution<int> len_name(5, 20);
    std::uniform_int_distribution<int> len_tags(3, 10);
    std::uniform_int_distribution<int> len_kv(3, 8);
    std::uniform_int_distribution<int> dc('a', 'z');

    Profile p;
    p.id = di(rng);
    p.score = dd(rng);

    int nname = len_name(rng);
    p.name.resize(nname);
    for (int i = 0; i < nname; ++i)
        p.name[i] = static_cast<char>(dc(rng));

    int ntags = len_tags(rng);
    p.tags.resize(ntags);
    for (int i = 0; i < ntags; ++i)
        p.tags[i] = di(rng);

    int nkv = len_kv(rng);
    for (int i = 0; i < nkv; ++i)
    {
        int klen = 5 + (di(rng) % 10);
        std::string key;
        key.resize(klen);
        for (int j = 0; j < klen; ++j)
            key[j] = static_cast<char>(dc(rng));
        p.kv.emplace(std::move(key), static_cast<int64_t>(di(rng)));
    }
    return p;
}

template <typename T>
static std::vector<T> MakeRandomVector(size_t n, uint64_t seed = 123456789ULL)
{
    std::mt19937_64 rng(seed);
    std::vector<T> v;
    v.reserve(n);
    for (size_t i = 0; i < n; ++i)
        v.emplace_back(MakeRandomProfile(rng));
    return v;
}

struct Dataset
{
    std::vector<Profile> many;

    std::string bin_many;
    std::string json_many;

    size_t bin_many_len{};
    size_t json_many_len{};
};

static Dataset &GetDataset()
{
    static Dataset ds = []
    {
        Dataset d;
        d.many = MakeRandomVector<Profile>(vectorSize, 424242);

        d.bin_many = Serializer::Serialize(d.many);

        {
            Json::StreamWriterBuilder builder;
            builder["indentation"] = "";
            Json::Value j;
            for (auto &p : d.many)
                j.append(ToJson(p));
            d.json_many = Json::writeString(builder, j);
        }

        d.bin_many_len = d.bin_many.size();
        d.json_many_len = d.json_many.size();

        std::cout << "Binary vector size: " << d.bin_many_len << "\n";
        std::cout << "JSON vector size:   " << d.json_many_len << "\n";

        return d;
    }();
    return ds;
}

static void BM_Custom_Serialize(benchmark::State &st)
{
    auto &ds = GetDataset();
    for (auto _ : st)
    {
        auto s = Serializer::Serialize(ds.many);
        benchmark::DoNotOptimize(s);
        benchmark::ClobberMemory();
    }
}

static void BM_JSON_Serialize(benchmark::State &st)
{
    auto &ds = GetDataset();
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    for (auto _ : st)
    {
        Json::Value arr;
        for (auto &p : ds.many)
            arr.append(ToJson(p));
        auto s = Json::writeString(builder, arr);
        benchmark::DoNotOptimize(s);
        benchmark::ClobberMemory();
    }
}

static void BM_Custom_Deserialize(benchmark::State &st)
{
    auto &ds = GetDataset();
    for (auto _ : st)
    {
        std::vector<Profile> v = Serializer::DeSerialize<std::vector<Profile>>(ds.bin_many);
        benchmark::DoNotOptimize(v);
        benchmark::ClobberMemory();
    }
}

static void BM_JSON_Deserialize(benchmark::State &st)
{
    auto &ds = GetDataset();
    Json::CharReaderBuilder builder;
    for (auto _ : st)
    {
        Json::Value j;
        std::string errs;
        std::istringstream s(ds.json_many);
        Json::parseFromStream(builder, s, &j, &errs);
        std::vector<Profile> v;
        v.reserve(j.size());
        for (auto &e : j)
            v.push_back(FromJson(e));
        benchmark::DoNotOptimize(v);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(BM_Custom_Serialize);
BENCHMARK(BM_JSON_Serialize);
BENCHMARK(BM_Custom_Deserialize);
BENCHMARK(BM_JSON_Deserialize);