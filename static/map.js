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
// Chargement des données
// ============================================

async function loadMapData() {
  try {
    const response = await fetch('map.json');
    if (!response.ok) {
      console.warn('Failed to load map.json:', response.status);
      return null;
    }
    return await response.json();
  } catch (e) {
    console.warn('Failed to fetch map.json:', e);
    return null;
  }
}

// ============================================
// Initialisation
// ============================================

async function initMap() {
  // Récupérer les données injectées par le générateur (fallback)
  // OU charger depuis map.json
  if (typeof window.SLOVINGO_MAP !== 'undefined') {
    MAP_STEPS = window.SLOVINGO_MAP.steps || [];
    MAP_RESOURCES = window.SLOVINGO_MAP.resources || [];
  } else {
    const mapData = await loadMapData();
    if (mapData) {
      MAP_STEPS = mapData.steps || [];
      MAP_RESOURCES = mapData.resources || [];
      window.SLOVINGO_MAP = mapData;
    }
  }

  // Charger la progression existante
  loadProgress();

  // Calculer l'état de la carte
  const mapState = getMapState(currentState.progress);

  // Rendre les éléments statiques
  renderNodes(MAP_STEPS, mapState);
  updateFoxPosition(mapState, false);
  updateMapVisibility(mapState);

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
  const isProgress = step.type === 'series' && currentState.progress[step.id] !== undefined && !isValidated;

  const classes = [
    'map-node',
    `map-node--${step.type}`,
  ];

  if (isValidated) {
    classes.push('is-validated');
  } else if (isCurrent) {
    classes.push('is-current');
  } else if (isProgress) {
    classes.push('is-in-progress');
  } else if (isDiscovered) {
    classes.push('is-discovered');
  } else {
    classes.push('is-future');
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

  // Afficher la progression si en cours (après le label)
  if (isProgress) {
    const progress = document.createElement('span');
    progress.className = 'map-node__progress';
    progress.textContent = `${currentState.progress[step.id]}%`;
    element.appendChild(progress);
  }

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
    if (mapState.validated.includes(step.id)) {
      parts.push(`${score}% completed`);
    } else {
      parts.push(`${score}% in progress`);
    }
  }

  // Ajouter l'état
  if (mapState.validated.includes(step.id)) {
    parts.push('completed');
  } else if (step.id === mapState.currentStep) {
    parts.push('current');
  } else if (mapState.discovered.includes(step.id)) {
    parts.push('discovered');
  } else {
    parts.push('locked');
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
// Mise à jour de la visibilité (fog, castle)
// ============================================

function updateMapVisibility(mapState) {
  // Révéler progressivement les zones de brouillard selon la dernière série validée
  const lastValidatedIndex = mapState.lastValidatedIndex;

  // Calculer le nombre de zones à révéler (une zone par 2 séries environ)
  const numSeries = MAP_STEPS.filter(s => s.type === 'series').length;
  const zonesPerSeries = 6 / Math.max(1, numSeries); // 6 fog zones au total
  const zonesToReveal = Math.ceil((lastValidatedIndex + 1) * zonesPerSeries);

  // Révéler les zones
  for (let i = 1; i <= 6; i++) {
    const fogZone = document.querySelector(`.fog-zone--0${i}`);
    if (fogZone) {
      if (i <= zonesToReveal) {
        fogZone.classList.add('is-revealed');
      } else {
        fogZone.classList.remove('is-revealed');
      }
    }
  }

  // Révéler le château si beaucoup de séries sont validées
  const castle = document.getElementById('map-castle');
  if (castle) {
    const allSeriesValidated = MAP_STEPS.filter(s => s.type === 'series').length > 0 &&
                               MAP_STEPS.filter(s => s.type === 'series' && mapState.validated.includes(s.id)).length ===
                               MAP_STEPS.filter(s => s.type === 'series').length;

    if (allSeriesValidated || lastValidatedIndex >= MAP_STEPS.filter(s => s.type === 'series').length - 2) {
      castle.classList.add('is-revealed');
    } else {
      castle.classList.remove('is-revealed');
    }
  }

  // Révéler les segments du chemin
  revealPathSegments(mapState);
}

// ============================================
// Révélation du chemin
// ============================================

function revealPathSegments(mapState) {
  // Pour la V1, révéler tous les segments jusqu'à la dernière étape validée
  // (Dans une future version, on peut animer les segments progressivement)

  const discoveredCount = mapState.discovered.length;

  // Chaque segment correspond à une étape
  for (let i = 0; i < discoveredCount; i++) {
    const segment = document.getElementById(`route-segment-${i}`);
    if (segment) {
      segment.classList.add('is-discovered');
    }
  }
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

  // Déterminer la prochaine étape recommandée
  const nextStep = MAP_STEPS.find(s => s.id === mapState.currentStep);

  if (nextStep) {
    // Placer le renard à la prochaine étape et animer
    currentState.lastVisited = nextStep.id;
    saveProgress();
    updateFoxPosition(mapState, true);
  }

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
  // Phase 1: Message de félicitation (optionnel, peut être dans les fiches)
  // Phase 2: Animation du renard (avec la transition CSS)
  // Phase 3: Révéler le brouillard et le chemin
  updateMapVisibility(mapState);

  // Phase 4: Mettre en évidence la nouvelle étape
  const nextNode = document.querySelector(`[data-step-id="${mapState.currentStep}"]`);
  if (nextNode) {
    nextNode.classList.add('is-arriving');
    setTimeout(() => {
      nextNode.classList.remove('is-arriving');
    }, 400); // Match animation duration
  }
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
