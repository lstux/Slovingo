/**
 * Slovingo Adventure Map
 * Étape 1 — Moteur de carte statique
 *
 * Responsabilités:
 * - Générer les nœuds du parcours à partir de MAP_STEPS
 * - Gérer l'état de progression (découvert/courant/validé)
 * - Positionner et animer le renard
 * - Révéler progressivement les zones de brouillard
 * - Gérer les popover d'information
 */

// Configuration globale
const MAP_CONFIG = {
  validationThreshold: 85,
  foxAnimationDuration: 1200, // ms
  discoveryAnimationDuration: 1500, // ms
  mobileBreakpoint: 700, // px
};

// Données du parcours (injectées par le générateur Python)
// TODO: Will be injected by build.py via window.SLOVINGO_MAP
let MAP_STEPS = [];
let MAP_RESOURCES = [];

// ============================================
// État local
// ============================================

let currentState = {
  progress: {},
  lastVisited: null,
};

// ============================================
// Initialisation
// ============================================

function initMap() {
  // Récupérer les données injectées par le générateur
  if (typeof window.SLOVINGO_MAP !== 'undefined') {
    MAP_STEPS = window.SLOVINGO_MAP.steps || [];
    MAP_RESOURCES = window.SLOVINGO_MAP.resources || [];
  }

  // Charger la progression existante
  loadProgress();

  // Calculer l'état de la carte
  const mapState = getMapState(currentState.progress);

  // Rendre les éléments statiques
  renderNodes(MAP_STEPS, mapState);
  updateFoxPosition(mapState, false);

  // Attacher les écouteurs d'événements
  attachEventListeners();

  // Écouter les événements de validation depuis les fiches
  if (typeof document !== 'undefined') {
    document.addEventListener('slovingo:series-validated', handleSeriesValidated);
  }
}

// ============================================
// Gestion de la progression
// ============================================

function loadProgress() {
  try {
    const stored = localStorage.getItem('slovingo_progress');
    if (stored) {
      currentState.progress = JSON.parse(stored);
    }

    const lastVisited = localStorage.getItem('slovingo_last_visited');
    if (lastVisited) {
      currentState.lastVisited = JSON.parse(lastVisited);
    }
  } catch (e) {
    console.warn('Failed to load progress:', e);
  }
}

function saveProgress() {
  try {
    localStorage.setItem('slovingo_progress', JSON.stringify(currentState.progress));
    localStorage.setItem('slovingo_last_visited', JSON.stringify(currentState.lastVisited));
  } catch (e) {
    console.warn('Failed to save progress:', e);
  }
}

function isSeriesValidated(seriesId) {
  const score = currentState.progress[seriesId];
  return score !== undefined && score >= MAP_CONFIG.validationThreshold;
}

// ============================================
// Calcul de l'état de la carte
// ============================================

function getMapState(progress) {
  const discovered = [];
  const validated = [];

  // Ajouter les étapes spéciales
  if (progress.intro || progress.intro === true) {
    discovered.push('intro');
  }
  if (progress.survival || progress.survival === true) {
    discovered.push('survival');
  }

  // Parcourir les séries
  let lastValidatedIndex = -1;

  for (let i = 0; i < MAP_STEPS.length; i++) {
    const step = MAP_STEPS[i];

    if (step.type === 'series' && isSeriesValidated(step.id)) {
      validated.push(step.id);
      discovered.push(step.id);
      lastValidatedIndex = i;
    }
  }

  // La prochaine étape recommandée est celle juste après la dernière validée
  const nextRecommended = lastValidatedIndex + 1 < MAP_STEPS.length
    ? MAP_STEPS[lastValidatedIndex + 1]
    : null;

  // Découvrir les étapes jusqu'à la prochaine recommandée + 1
  if (nextRecommended) {
    const nextIndex = MAP_STEPS.indexOf(nextRecommended);
    for (let i = 0; i <= nextIndex + 1 && i < MAP_STEPS.length; i++) {
      discovered.push(MAP_STEPS[i].id);
    }
  } else {
    // Tout découvert
    MAP_STEPS.forEach(step => discovered.push(step.id));
  }

  return {
    currentStep: nextRecommended?.id || MAP_STEPS[0]?.id,
    discovered,
    validated,
    lastValidatedIndex,
  };
}

// ============================================
// Rendu des nœuds
// ============================================

function renderNodes(steps, mapState) {
  const container = document.getElementById('map-nodes');
  if (!container) return;

  container.innerHTML = '';

  for (const step of steps) {
    const node = createMapNode(step, mapState);
    container.appendChild(node);
  }
}

function createMapNode(step, mapState) {
  const isDiscovered = mapState.discovered.includes(step.id);
  const isValidated = mapState.validated.includes(step.id);
  const isCurrent = step.id === mapState.currentStep;

  const classes = [
    'map-node',
    `map-node--${step.type}`,
  ];

  if (isDiscovered) {
    classes.push('is-discovered');
  } else {
    classes.push('is-future');
  }

  if (isValidated) {
    classes.push('is-validated');
  }

  if (isCurrent) {
    classes.push('is-current');
  }

  const element = document.createElement('button');
  element.className = classes.join(' ');
  element.style.left = `${step.x}%`;
  element.style.top = `${step.y}%`;
  element.setAttribute('data-step-id', step.id);
  element.setAttribute('aria-label', getNodeAriaLabel(step, mapState));

  // Contenu du nœud
  const icon = document.createElement('span');
  icon.className = 'map-node__icon';
  icon.textContent = step.icon || '✈️';

  const label = document.createElement('span');
  label.className = 'map-node__label';
  label.textContent = step.title || '';

  element.appendChild(icon);
  element.appendChild(label);

  // Événement au clic
  element.addEventListener('click', (e) => handleNodeClick(e, step, mapState));

  return element;
}

function getNodeAriaLabel(step, mapState) {
  const parts = [step.title];

  if (step.label) {
    parts.push(step.label);
  }

  if (step.type === 'series' && currentState.progress[step.id]) {
    const score = currentState.progress[step.id];
    parts.push(`${score}% progress`);
  }

  if (mapState.validated.includes(step.id)) {
    parts.push('completed');
  }

  return parts.join(', ');
}

// ============================================
// Gestion du renard
// ============================================

function updateFoxPosition(mapState, animate = false) {
  const fox = document.getElementById('map-fox');
  if (!fox) return;

  let position;

  if (currentState.lastVisited) {
    // Placer le renard à la dernière consultation
    position = getPositionForStep(currentState.lastVisited);
  } else {
    // Premier chargement : renard au départ
    const firstStep = MAP_STEPS[0];
    position = getPositionForStep(firstStep?.id);
  }

  if (!position) {
    position = { x: 12, y: 88 }; // Fallback
  }

  // Mettre à jour les variables CSS
  if (animate) {
    fox.classList.add('is-arriving');
    setTimeout(() => {
      fox.classList.remove('is-arriving');
    }, 600);
  }

  fox.style.setProperty('--fox-x', `${position.x}%`);
  fox.style.setProperty('--fox-y', `${position.y}%`);
}

function getPositionForStep(stepId) {
  const step = MAP_STEPS.find(s => s.id === stepId);
  if (!step) return null;

  return {
    x: step.x,
    y: step.y,
  };
}

// ============================================
// Gestion des popovers
// ============================================

function showNodePopover(step, mapState) {
  const popover = document.getElementById('map-popover');
  if (!popover) return;

  document.getElementById('popover-title').textContent = step.title;
  document.getElementById('popover-label').textContent = step.label || '';

  // Afficher la progression si applicable
  const progressDiv = document.getElementById('popover-progress');
  if (step.type === 'series' && currentState.progress[step.id]) {
    const score = currentState.progress[step.id];
    const isValidated = mapState.validated.includes(step.id);
    progressDiv.textContent = `${score}% ${isValidated ? '✓ completed' : ''}`;
    progressDiv.style.display = 'block';
  } else {
    progressDiv.style.display = 'none';
  }

  // Configurer le bouton d'action
  const actionBtn = document.getElementById('popover-action');
  if (step.href) {
    actionBtn.href = step.href;
    actionBtn.textContent = mapState.discovered.includes(step.id)
      ? 'Continue'
      : 'Explore';
  }

  // Afficher le popover
  popover.removeAttribute('hidden');
  popover.setAttribute('aria-modal', 'true');
}

function hideNodePopover() {
  const popover = document.getElementById('map-popover');
  if (popover) {
    popover.setAttribute('hidden', '');
  }
}

// ============================================
// Gestion des événements
// ============================================

function handleNodeClick(event, step, mapState) {
  event.preventDefault();

  // Enregistrer la dernière consultation
  currentState.lastVisited = step.id;
  saveProgress();

  // Afficher le popover
  showNodePopover(step, mapState);

  // Naviguer vers le contenu après un délai court
  // (permettre au popover de s'afficher d'abord)
  setTimeout(() => {
    if (step.href) {
      window.location.href = step.href;
    }
  }, 300);
}

function handleSeriesValidated(event) {
  const { seriesId, score } = event.detail;

  // Mettre à jour la progression
  currentState.progress[seriesId] = score;
  saveProgress();

  // Recalculer l'état et rendre la carte
  const mapState = getMapState(currentState.progress);
  renderNodes(MAP_STEPS, mapState);

  // Animer la révélation
  animateDiscovery(seriesId, mapState);
}

function attachEventListeners() {
  // Fermer le popover
  const popoverClose = document.querySelector('.popover-close');
  if (popoverClose) {
    popoverClose.addEventListener('click', hideNodePopover);
  }

  // Fermer le popover si on clique en dehors
  const popover = document.getElementById('map-popover');
  if (popover) {
    popover.addEventListener('click', (e) => {
      if (e.target === popover) {
        hideNodePopover();
      }
    });
  }
}

// ============================================
// Animations
// ============================================

function animateDiscovery(seriesId, mapState) {
  // TODO: Implémenter l'animation de découverte
  // - Phase 1: Afficher un message de félicitation
  // - Phase 2: Animation du renard
  // - Phase 3: Illuminer le chemin
  // - Phase 4: Révéler la brume
  // - Phase 5: Déplacer le renard
  // - Phase 6: Mettre à jour l'étape courante
}

// ============================================
// Accès direct aux ressources
// ============================================

function handleDirectResourceClick(event, resourceId) {
  event.preventDefault();
  // TODO: Implémenter la navigation vers les ressources
  // La ressource dépendra de MAP_RESOURCES et du config de langue
}

// ============================================
// Démarrage
// ============================================

if (document.readyState === 'loading') {
  document.addEventListener('DOMContentLoaded', initMap);
} else {
  initMap();
}
