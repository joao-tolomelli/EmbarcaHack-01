// src/server.js
import express from 'express';
import cors from 'cors';
import dotenv from 'dotenv';
import path from 'path';
import { fileURLToPath } from 'url';
import apiRoutes from './routes/api.routes.js';

import swaggerUi from 'swagger-ui-express';
import YAML from 'yamljs';

dotenv.config();

const app = express();
const PORT = process.env.PORT || 3000;

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const swaggerDocument = YAML.load(path.join(__dirname, '../swagger/openapi-medicamentos.yaml'));

// rota de documentação
app.use('/api-docs', swaggerUi.serve, swaggerUi.setup(swaggerDocument));

// Middlewares
app.use(cors());
app.use(express.json());

// Importar e montar rotas da API
// Montar rotas da API
app.use('/api/v1', apiRoutes);        // Mantemos a anterior

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

import pool from './config/db.js';

pool.query('SELECT 1')
  .then(() => console.log('✅ Conexão com MySQL bem-sucedida'))
  .catch(err => console.error('❌ Erro ao conectar com MySQL:', err));

// Inicializar servidor
app.listen(PORT, () => {
  console.log(`🚀 Servidor rodando em http://localhost:${PORT}`);
});
