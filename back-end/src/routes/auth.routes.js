// src/routes/auth.routes.js
import express from 'express';
import { register, login } from '../controllers/auth.controller.js';
import jwtMiddleware from '../middleware/jwt.middleware.js';

const router = express.Router();

// Rota de registro
router.post('/register', register);

// Rota de login
router.post('/login', login);

// Rota protegida que retorna os dados do usuário logado
router.get('/me', jwtMiddleware, (req, res) => {
  const { id, email, profile } = req.user;
  res.json({ id, email, profile });
});

export default router;
