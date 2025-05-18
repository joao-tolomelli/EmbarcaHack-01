// src/routes/api.routes.js
import express from 'express';
import jwtMiddleware from '../middleware/jwt.middleware.js';
import authRoutes from './auth.routes.js';
import medicamentoRoutes from './medicamento.routes.js';

const router = express.Router();

// Rota pública de status
router.get('/status', (req, res) => {
  res.json({ status: 'API está online', version: '0.1' });
});

// Rota protegida de teste
router.get('/private', jwtMiddleware, (req, res) => {
  res.json({
    message: 'Acesso autorizado à rota protegida!',
    user: req.user
  });
});

// Montar sub-rotas de autenticação e medicamentos
router.use('/auth', authRoutes);
router.use('/medicamentos', medicamentoRoutes);

export default router;
