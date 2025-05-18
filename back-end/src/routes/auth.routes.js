// src/routes/auth.routes.js
import express from 'express';
import { register, login } from '../controllers/auth.controller.js';
import jwtMiddleware from '../middleware/jwt.middleware.js';
import { validate } from '../middleware/validate.middleware.js';
import { registerSchema, loginSchema } from '../validators/auth.validator.js';

const router = express.Router();

// Rota de registro
router.post('/register', validate(registerSchema), register);

// Rota de login
router.post('/login', validate(loginSchema), login);

// Rota protegida que retorna os dados do usuário logado
router.get('/me', jwtMiddleware, (req, res) => {
  const { id, email, profile } = req.user;
  res.json({ id, email, profile });
});

export default router;
