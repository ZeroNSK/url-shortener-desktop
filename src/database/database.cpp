#include "database.h"
#include "../external/json.hpp"    // Библиотека nlohmann/json для работы с JSON
#include <fstream>
#include <sstream>
#include <iostream>
#include <functional>
#include <vector>

using json = nlohmann::json;
using namespace std;

bool Database::load(const string& path) {

    ifstream f(path);
    if (!f) return true; // если файла нет, создаем пустую БД

    json j; 
    try {
        f >> j;
    } catch (...) {
        return false; 
    }
    if (j.contains("sets")) {
        for (auto& [name, arr] : j["sets"].items()) { // Проходим по всем множествам

            Set s;

            // Добавляем все элементы из JSON массива
            for (auto& v : arr) {
                s.add(v.get<string>());
            }

            sets[name] = s; // Сохраняем множество в базе данных
        }
    }

    // стек 
    if (j.contains("stacks")) { 
        for (auto& [name, arr] : j["stacks"].items()) { // тут все то же самое, но для стека

            Stack st;

            // стек загружаем по принципу last --> first
            for (auto it = arr.rbegin(); it != arr.rend(); ++it) {
                st.push(it->get<string>());
            }

            stacks[name] = st;
        }
    }

    // очередь
    if (j.contains("queues")) {
        for (auto& [name, arr] : j["queues"].items()) { // тут все то же самое, но для очереди

            Queue q;

            for (auto& v : arr) {
                q.push(v.get<string>());
            }

            queues[name] = q;
        }
    }

    // хэш таблица
    if (j.contains("hashes")) {
        for (auto& [name, obj] : j["hashes"].items()) { // тут все то же самое, но для хэш таблицы

            Hash h;

            for (auto& [key, value] : obj.items()) {
                h.set(key, value.get<string>());
            }

            hashes[name] = h;
        }
    }

    // бст
    if (j.contains("trees")) {
        for (auto& [name, arr] : j["trees"].items()) {

            BinaryTree t;
            
            if (!arr.empty()) {
                vector<string> data;
                for (auto& v : arr) {
                    data.push_back(v.get<string>());
                }
                int index = 0;
                t.root = t.deserializeFromArray(data, index);
            }

            trees[name] = t;
        }
    }

    return true;
}


bool Database::save(const string& path) {

    json j;

    // множество
    for (auto& [name, s] : sets) { // проходимся по всем множествам

        json arr = json::array(); // каждое множество - массив

        for (int i = 0; i < SET_BUCKETS; i++) {
            SetNode* cur = s.buckets[i]; // смотрим каждый элемент в корзине
            while (cur) { 
                arr.push_back(cur->value); // доавляем в массив пока есть элементы 
                cur = cur->next;
            }
        }

        j["sets"][name] = arr;
    }

    // стек
    for (auto& [name, st] : stacks) {

        json arr = json::array();

        StackNode* cur = st.top; // начинаем с вершины стека
        while (cur) {
            arr.push_back(cur->value);
            cur = cur->next;
        }

        j["stacks"][name] = arr;
    }

    // очередь
    for (auto& [name, q] : queues) {

        json arr = json::array();

        QueueNode* cur = q.head; // начинаем с головы очереди
        while (cur) {
            arr.push_back(cur->value);
            cur = cur->next;
        }

        j["queues"][name] = arr;
    }

    // хэш таблица
    for (auto& [name, h] : hashes) {

        json obj = json::object();

        for (int i = 0; i < 1024; i++) {
            HashNode* cur = h.buckets[i];
            while (cur) {
                obj[cur->key] = cur->value;
                cur = cur->next;
            }
        }

        j["hashes"][name] = obj;
    }

    //бст
    for (auto& [name, t] : trees) {

        json arr = json::array();

        vector<string> data;
        t.serializeToArray(t.root, data);
        
        for (const auto& value : data) {
            arr.push_back(value);
        }
        j["trees"][name] = arr;
    }

    ofstream out(path);
    if (!out) return false; // если не удалось открыть файл для записи

    out << j.dump(4);     // красиво форматируем с отступами
    return true;
}