import { Routes } from '@angular/router';
import { Profile } from './profile/profile';

export const routes: Routes = [
  // Fontos: itt ne legyen per jel az út elején!
  { path: 'profile', component: Profile }
];