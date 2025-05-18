// src/middleware/role.middleware.js

/**
 * Middleware que permite apenas perfis autorizados acessarem uma rota.
 * @param {Array<number>} allowedProfiles - Códigos dos perfis permitidos.
 */
export function permitOnly(...allowedProfiles) {
  return (req, res, next) => {
    const user = req.user;

    if (!user || !allowedProfiles.includes(user.profile)) {
      return res.status(403).json({ message: 'Acesso negado: perfil não autorizado' });
    }

    next();
  };
}
