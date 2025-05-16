// routes/auth.router.js
import { Router } from 'express';
import {
  loginUser,
  registerUser,
  getUserProfile
} from '../controllers/auth.controller.js';
import { verifyToken } from '../middleware/auth.middleware.js';

const router = Router();

// Rota pública para login
router.post('/login', loginUser);

// Rota pública para registro
router.post('/register', registerUser);

// Rota protegida para dados do usuário autenticado
router.get('/me', verifyToken, getUserProfile);

export default router;
