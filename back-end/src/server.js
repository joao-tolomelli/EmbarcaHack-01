// src/server.js
import express from 'express';
import cors from 'cors';
import dotenv from 'dotenv';
import path from 'path';
import { fileURLToPath } from 'url';

dotenv.config();

const app = express();
const PORT = process.env.PORT || 3000;

// Suporte a __dirname com ESModules
const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Middlewares
app.use(cors());
app.use(express.json());

// Importar e montar rotas da API
import apiRoutes from './routes/api.routes.js';
app.use('/api/v0.1', apiRoutes);

// Servir arquivos estáticos do frontend
const frontendPath = path.resolve(__dirname, '../../front-end/dist');
app.use(express.static(frontendPath));

// Fallback para index.html para rotas não resolvidas (SPA)
app.use((req, res, next) => {
  // Se a requisição não começar com /api
  if (!req.path.startsWith('/api')) {
    return res.sendFile(path.join(frontendPath, 'index.html'));
  }
  // Senão, continua e envia 404 para APIs inexistentes
  return res.status(404).json({ message: 'Endpoint não encontrado' });
});

// Inicializar servidor
app.listen(PORT, () => {
  console.log(`🚀 Servidor rodando em http://localhost:${PORT}`);
});
