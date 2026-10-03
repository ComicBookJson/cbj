#pragma once

#include <string>
#include <vector>
#include "CbjzSchema.hpp" // Seu esquema gerado pelo cbjz

namespace cbjz {

    class Cbj {
    public:
        Cbj();
        ~Cbj();

        // Evitar cópias indevidas do SDK na memória
        Cbj(const Cbj&) = delete;
        Cbj& operator=(const Cbj&) = delete;

        // ==========================================
        // 1. LEITURA DE FORMATOS (Abertura / Importação)
        // ==========================================
        
        // Abre um arquivo .cbjz original. Não carrega as imagens na memória!
        // Apenas carrega o Metadata e o índice das páginas.
        bool OpenCbjz(const std::string& filePath);

        // Importadores: Lê formatos de terceiros para conversão em memória
        bool ImportFromCbz(const std::string& filePath);
        bool ImportFromCbr(const std::string& filePath);
        bool ImportFromPdf(const std::string& filePath);

        // ==========================================
        // 2. EXPORTAÇÃO
        // ==========================================
        
        // Salva o estado atual (páginas importadas ou editadas) em um novo .cbjz
        bool SaveAsCbjz(const std::string& outputPath);

        // ==========================================
        // 3. LEITURA EM PARTES (PAGINAÇÃO / STREAMING)
        // ==========================================
        
        // Lê 1, 2 ou 3 páginas a partir de um índice.
        // Internamente, usará um SAX Parser para ler apenas os blocos Base64 
        // necessários do disco, mantendo a RAM baixa.
        std::vector<cbjz::PageElement> GetPages(int startIndex, int count);
        
        // Atalho para pegar uma única página
        cbjz::PageElement GetPage(int index);

        // ==========================================
        // 4. EDIÇÃO E METADADOS
        // ==========================================
        
        cbjz::Metadata GetMetadata() const;
        void SetMetadata(const cbjz::Metadata& metadata);

        // Atualiza as tags ou summary de uma página específica
        void UpdatePage(int index, const cbjz::PageElement& updatedPage);

        int GetTotalPages() const;
        std::string GetVersion() const;

    private:
        // Padrão PIMPL (Pointer to Implementation)
        // Esconde todas as variáveis de estado, streamings e bibliotecas 
        // pesadas (libzip, unrar, pdfium, nlohmann) fora desse arquivo.
        class Impl;
        Impl* pImpl;
    };

}