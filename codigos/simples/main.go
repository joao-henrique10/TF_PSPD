package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"log"
	"net/http"
	"strings"
)

func countWords(text string, wordCounts map[string]int) map[string]int {
	words := strings.Fields(text)
	for _, word := range words {
		wordCounts[word]++
	}

	return wordCounts
}

func wordCountHandler(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Invalid request method", http.StatusMethodNotAllowed)
		return
	}

	wordCounts := make(map[string]int)
	scanner := bufio.NewScanner(r.Body)
	defer r.Body.Close()

	for scanner.Scan() {
		line := scanner.Text()
		countWords(line, wordCounts)
	}

	response := make(map[string]interface{})
	response["word_counts"] = wordCounts

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(response)
}

func main() {
	http.HandleFunc("/", wordCountHandler)

	fmt.Println("Server is running on port 8080")
	log.Fatal(http.ListenAndServe(":8080", nil))
}
