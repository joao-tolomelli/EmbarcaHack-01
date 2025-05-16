// src/middleware/jwt.middleware.js
import jwt from 'jsonwebtoken';

const jwtMiddleware = (req, res, next) => {
  const authHeader = req.headers['authorization'];
  
  if (!authHeader || !authHeader.startsWith('Bearer ')) {
    return res.status(401).json({ message: 'Token não fornecido ou mal formatado' });
  }

  const token = authHeader.split(' ')[1];

  try {
    const decoded = jwt.verify(token, process.env.JWT_SECRET);
    req.user = decoded; // Anexa os dados do token à requisição
    next(); // Prossegue para a próxima função
  } catch (err) {
    return res.status(401).json({ message: 'Token inválido ou expirado' });
  }
};

export default jwtMiddleware;
