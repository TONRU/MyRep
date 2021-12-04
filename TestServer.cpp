void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    // —начала убеждаемс€, что поиск слова, не вход€щего в список стоп-слов,
    // находит нужный документ
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);

    }

    // «атем убеждаемс€, что поиск этого же слова, вход€щего в список стоп-слов,
    // возвращает пустой результат
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(), "Stop words must be excluded from documents"s);
    }
}

void TestAddDocument() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };

    const int doc_id2 = 43;
    const string content2 = "white collar fluffy cat"s;
    const vector<int> ratings2 = { 4,7,9 };

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto result = server.FindTopDocuments("city"s);
        ASSERT_EQUAL(result.size(), 1u);
        ASSERT_EQUAL(result[0].id, doc_id);

    }
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto result = server.FindTopDocuments("dog"s);
        ASSERT(result.empty());
    }


}

void TestMatchWords() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };

    const int doc_id2 = 43;
    const string content2 = "white collar fluffy cat"s;
    const vector<int> ratings2 = { 4,7,9 };

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);

        const auto [words, status] = server.MatchDocument("fluffy cat"s, doc_id2);
        ASSERT_EQUAL(words.size(), 2u);
        ASSERT((words[0] == "cat"s && words[1] == "fluffy"s) || (words[1] == "cat"s && words[0] == "fluffy"s));

    }

    {
        SearchServer server;
        server.SetStopWords("and in on the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto [words, status] = server.MatchDocument("fluffy -cat"s, doc_id2);
        ASSERT_HINT(words.empty(), "No minus words");
    }

}


void TestMinusWords() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };

    const int doc_id2 = 43;
    const string content2 = "white collar fluffy cat"s;
    const vector<int> ratings2 = { 4,7,9 };


    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto result = server.FindTopDocuments("cat -city"s);
        ASSERT_EQUAL(result.size(), 1u);
        ASSERT_EQUAL(result[0].id, doc_id2);
    }
}
void TestRelevance() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };

    const int doc_id2 = 43;
    const string content2 = "dog in the city"s;
    const vector<int> ratings2 = { 4,5,6 };

    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        server.AddDocument(doc_id2, content2, DocumentStatus::ACTUAL, ratings2);
        const auto result = server.FindTopDocuments("city cat");
        ASSERT_EQUAL(result.size(), 2u);
        ASSERT_EQUAL(result[0].id, doc_id);
        for (int i = 1; i < result.size(); i++) {
            ASSERT_HINT(result[i - 1].relevance > result[i].relevance, "Wrong relevance"s);
        }

    }
}

void TestRating() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };

    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto result = server.FindTopDocuments("city cat"s);
        ASSERT_EQUAL(result[0].rating, 2);
    }
}


void TestRelevanceCalc() {
    SearchServer search_server;
    search_server.AddDocument(0, "cat in the city"s, DocumentStatus::ACTUAL, { 1,2,3 });
    search_server.AddDocument(1, "pig in the sky"s, DocumentStatus::ACTUAL, { 4,5,6 });
    search_server.AddDocument(2, "fluffy cat"s, DocumentStatus::ACTUAL, { 7,-8,9 });
    vector<Document> result = search_server.FindTopDocuments("cat"s);
    const double IDF = log(3 * 0.5);
    const double TF1 = 0.25;
    const double TF2 = 0.5;
    ASSERT_EQUAL_HINT(result[0].relevance, IDF * TF2, "Wrong calculation"s);
    ASSERT_EQUAL_HINT(result[1].relevance, IDF * TF1, "Wrong calculation"s);
}

void TestStatus() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };

    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::BANNED, ratings);
    const auto document = server.FindTopDocuments("cat", DocumentStatus::BANNED);
    ASSERT_EQUAL(document.size(), 1u);
    ASSERT_EQUAL_HINT(document[0].id, doc_id, "Wrong status search work"s);

}
void TestPredicate() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };

    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    const auto document = server.FindTopDocuments("cat"s, [](int document_id,
        DocumentStatus status, int rating) { return document_id % 2 == 0; });
    ASSERT_EQUAL(document.size(), 1u);
    ASSERT_EQUAL_HINT(document[0].id, doc_id, "TestPredicate isn't working"s);

}

void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestMatchWords);
    RUN_TEST(TestAddDocument);
    RUN_TEST(TestMinusWords);
    RUN_TEST(TestRating);
    RUN_TEST(TestRelevanceCalc);
    RUN_TEST(TestRelevance);
    RUN_TEST(TestStatus);
    RUN_TEST(TestPredicate);
}
