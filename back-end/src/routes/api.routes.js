// src/routes/api.routes.js
import express from 'express';
import jwtMiddleware from '../middleware/jwt.middleware.js';

const router = express.Router();

// Rota pública para verificação de status da API
router.get('/status', (req, res) => {
  res.json({ status: 'API está online', version: '0.1' });
});

// Rota protegida de exemplo
router.get('/private', jwtMiddleware, (req, res) => {
  res.json({
    message: 'Acesso autorizado à rota protegida!',
    user: req.user // incluído pelo middleware
  });
});

export default router;
