package main

import (
	"github.com/gorilla/mux"
	"net/http"
)

func getHandler(w http.ResponseWriter, r *http.Request) {
	w.Write([]byte("random value"))
}
func main() {
	r := mux.NewRouter()
	r.HandleFunc("/get/{key}", getHandler)
	if err := http.ListenAndServe(":8080", r); err != nil {
		panic(err)
	}
}
